// TeleportAbilityComponent.cpp

#include "TeleportAbilityComponent.h"
#include "CPP_TeleportShard.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/MovementComponent.h"
#include "PaperCharacter.h"
#include "PaperFlipbookComponent.h"
#include "PaperGroupedSpriteComponent.h"
#include "PaperSpriteComponent.h"
#include "PaperSprite.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h" // Required for teleport particles

// ------------------------------------------------------------------ Construction / destruction

UTeleportAbilityComponent::UTeleportAbilityComponent()
{
	// Ticks ONLY while aiming, to redraw the trajectory preview every frame.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PostPhysics; // after the character has moved this frame
}

// Called when the game starts, or when spawned, or when BeginPlay is called after spawning with FinishSpawning
void UTeleportAbilityComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clear timers to avoid callbacks after the component is gone.
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AimCycleTimer);
		World->GetTimerManager().ClearTimer(CooldownTimer);
	}

	EndHitStop();

	// Never leave enemies frozen if the player is removed mid-stun.
	TArray<TWeakObjectPtr<AActor>> Stunned;
	StunnedEnemies.GetKeys(Stunned);
	for (const TWeakObjectPtr<AActor>& Enemy : Stunned)
	{
		UnstunEnemy(Enemy);
	}

	// Remove the shard quietly (no fizzle event, no fizzle cooldown).
	if (ACPP_TeleportShard* Projectile = ActiveProjectile.Get())
	{
		Projectile->OnDestroyed.RemoveAll(this);
		Projectile->OnArmed.RemoveAll(this);
		Projectile->OnHitEnemy.RemoveAll(this);
		Projectile->Destroy();
	}
	ActiveProjectile.Reset();

	// Remove the preview components if they exist. They are not subobjects, so we destroy them manually.
	if (PreviewSprites)
	{
		PreviewSprites->DestroyComponent();
		PreviewSprites = nullptr;
	}
	if (PreviewLandingMarker)
	{
		PreviewLandingMarker->DestroyComponent();
		PreviewLandingMarker = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}


// ------------------------------------------------------------------ Input

void UTeleportAbilityComponent::OnTeleportInputPressed()
{
	switch (State)
	{
		// Start aiming if the ability is ready and unlocked. Otherwise, deny the input.
	case ETeleportAbilityState::Ready:
		if (bUnlocked)
		{
			StartAiming();
		}
		else
		{
			OnTeleportDenied.Broadcast();
		}
		break;

		// If aiming, throw the shard immediately (no need to wait for release).
	case ETeleportAbilityState::Armed:
		TryTeleport();
		break;

	case ETeleportAbilityState::InFlight:  // hasn't touched a surface yet
	case ETeleportAbilityState::Cooldown:
		OnTeleportDenied.Broadcast();
		break;

	default:
		break;
	}
}


// ------------------------------------------------------------------ Input release

void UTeleportAbilityComponent::OnTeleportInputReleased()
{
	// If the player releases the input while aiming, throw the shard. If they release it in any other state, do nothing.
	if (State == ETeleportAbilityState::Aiming)
	{
		ThrowProjectile();
	}
}


// ------------------------------------------------------------------ Unlock / cancel

void UTeleportAbilityComponent::SetUnlocked(bool bNewUnlocked)
{
	bUnlocked = bNewUnlocked;
	if (!bUnlocked)
	{
		CancelAim();
	}
}


// ------------------------------------------------------------------ Cancel / reset
void UTeleportAbilityComponent::CancelAim()
{
	// Only cancel if the player is currently aiming. If they are in any other state, do nothing.
	if (State != ETeleportAbilityState::Aiming)
	{
		return;
	}
	GetWorld()->GetTimerManager().ClearTimer(AimCycleTimer);
	StopPreview();
	SetState(ETeleportAbilityState::Ready);
	OnAimCancelled.Broadcast();
}

void UTeleportAbilityComponent::ResetAbility(bool bClearCooldown)
{
	// Reset the ability to its initial state, canceling any aim, removing the projectile, and optionally clearing the cooldown.
	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().ClearTimer(AimCycleTimer);
	}
	StopPreview();
	EndHitStop();

	// Remove the shard quietly (no fizzle event, no fizzle cooldown).
	if (ACPP_TeleportShard* Projectile = ActiveProjectile.Get())
	{
		Projectile->OnDestroyed.RemoveAll(this);
		Projectile->OnArmed.RemoveAll(this);
		Projectile->OnHitEnemy.RemoveAll(this);
		Projectile->Destroy();
	}
	ActiveProjectile.Reset();

	// Unstun any enemies that are still stunned.
	if (bClearCooldown || State != ETeleportAbilityState::Cooldown)
	{
		if (World)
		{
			World->GetTimerManager().ClearTimer(CooldownTimer);
		}
		SetState(ETeleportAbilityState::Ready);
	}

	OnAbilityReset.Broadcast();
}


// ------------------------------------------------------------------ Throw angle

float UTeleportAbilityComponent::GetCurrentThrowAngle() const
{
	return ThrowAnglesDegrees.IsValidIndex(AimIndex) ? ThrowAnglesDegrees[AimIndex] : 0.f;
}


// ------------------------------------------------------------------ Cooldown

float UTeleportAbilityComponent::GetCooldownRemaining() const
{
	const UWorld* World = GetWorld();
	if (!World || !World->GetTimerManager().IsTimerActive(CooldownTimer))
	{
		return 0.f;
	}
	return FMath::Max(0.f, World->GetTimerManager().GetTimerRemaining(CooldownTimer));
}

float UTeleportAbilityComponent::GetCooldownFractionRemaining() const
{
	return CooldownDuration > 0.f ? FMath::Clamp(GetCooldownRemaining() / CooldownDuration, 0.f, 1.f) : 0.f;
}


// ------------------------------------------------------------------ Facing

void UTeleportAbilityComponent::SetFacingDirection(float Direction)
{
	// 0 = no input this frame: keep facing the last direction.
	if (!FMath::IsNearlyZero(Direction))
	{
		FacingSign = Direction > 0.f ? 1.f : -1.f;
		bHasExplicitFacing = true;
	}
}

// ------------------------------------------------------------------ Throw direction

FVector UTeleportAbilityComponent::GetThrowForwardDirection_Implementation() const
{
	const AActor* OwnerActor = GetOwner();
	if (!OwnerActor)
	{
		return FVector::ForwardVector;
	}

	// The level's "right" direction. The character moves along world X (see
	// ACPP_PlayerChar::DoMove), so use world +X directly. This stays correct even
	// if the actor's rotation changes (e.g. Orient Rotation To Movement), which
	// would otherwise double-flip the throw.
	const FVector Right = FVector::ForwardVector;

	float Sign = 1.f;
	if (bHasExplicitFacing)
	{
		// Set by the character every time it turns (SetFacingDirection).
		Sign = FacingSign;
	}
	else
	{
		// Fallback: read the sign of the X scale (actor root and sprite).
		Sign = OwnerActor->GetActorScale3D().X < 0.f ? -1.f : 1.f;
		if (const APaperCharacter* PaperChar = Cast<APaperCharacter>(OwnerActor))
		{
			if (const UPaperFlipbookComponent* Sprite = PaperChar->GetSprite())
			{
				if (Sprite->GetRelativeScale3D().X < 0.f)
				{
					Sign *= -1.f;
				}
			}
		}
		if (bSpriteArtFacesLeft)
		{
			Sign *= -1.f;
		}
	}

	return Right * Sign;
}


// ------------------------------------------------------------------ Core flow

void UTeleportAbilityComponent::SetState(ETeleportAbilityState NewState)
{
	// If the state is not changing, do nothing. Otherwise, update the state and broadcast the change.
	if (NewState == State)
	{
		return;
	}
	const ETeleportAbilityState OldState = State;
	State = NewState;
	OnStateChanged.Broadcast(NewState, OldState);
}


// ------------------------------------------------------------------ Aiming

void UTeleportAbilityComponent::StartAiming()
{
	// Only start aiming if the ability is ready and unlocked. If it is in any other state, do nothing.
	if (!ProjectileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("TeleportAbilityComponent on %s has no ProjectileClass set."), *GetNameSafe(GetOwner()));
		return;
	}
	if (ThrowAnglesDegrees.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("TeleportAbilityComponent on %s has no ThrowAnglesDegrees."), *GetNameSafe(GetOwner()));
		return;
	}

	AimIndex = 0;
	AimStep = 1;
	SetState(ETeleportAbilityState::Aiming);
	OnAimStarted.Broadcast();
	OnAimIndexChanged.Broadcast(AimIndex);
	PlayAimChangedSound();

	// If there are multiple angles, start cycling through them automatically.
	if (ThrowAnglesDegrees.Num() > 1)
	{
		GetWorld()->GetTimerManager().SetTimer(AimCycleTimer, this, &UTeleportAbilityComponent::AdvanceAim, AimCycleInterval, true);
	}

	// Show the trajectory preview if enabled. The first update is done immediately, not one frame later.
	if (bShowTrajectoryPreview)
	{
		SetComponentTickEnabled(true);
		TimeSincePreviewUpdate = 0.f;
		UpdateTrajectoryPreview(true); // show it immediately, not one frame later
	}
}


// ------------------------------------------------------------------ Aim cycling

// Play a sound when the aiming angle changes, using the assigned projectile Blueprint defaults.
void UTeleportAbilityComponent::PlayAimChangedSound() const
{
	if (!ProjectileClass) return;

	const ACPP_TeleportShard* ShardDefaults = ProjectileClass->GetDefaultObject<ACPP_TeleportShard>();
	if (ShardDefaults && ShardDefaults->GetAimChangedSound())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), ShardDefaults->GetAimChangedSound());
	}
}

void UTeleportAbilityComponent::AdvanceAim()
{
	// If there is only one angle, do nothing. Otherwise, advance the aim index to the next angle, wrapping around if necessary.
	const int32 Num = ThrowAnglesDegrees.Num();
	if (Num <= 1)
	{
		return;
	}

	// If ping-pong cycling is enabled, reverse direction when reaching the end of the list. Otherwise, wrap around to the beginning.
	if (bPingPongAimCycle)
	{
		int32 Next = AimIndex + AimStep;
		if (Next < 0 || Next >= Num)
		{
			AimStep = -AimStep;
			Next = AimIndex + AimStep;
		}
		AimIndex = Next;
	}
	else
	{
		AimIndex = (AimIndex + 1) % Num;
	}

	OnAimIndexChanged.Broadcast(AimIndex);
	PlayAimChangedSound();

	// If the player is still aiming and the trajectory preview is enabled, update the preview immediately to reflect the new angle.
	if (State == ETeleportAbilityState::Aiming && bShowTrajectoryPreview)
	{
		UpdateTrajectoryPreview(true); // new angle: update right away
	}
}


// ------------------------------------------------------------------ Throw calculations

bool UTeleportAbilityComponent::ComputeLaunch(FVector& OutSpawnLocation, FVector& OutVelocity, FVector& OutPlaneNormal) const
{
	// Compute the spawn location, launch velocity, and plane normal for the teleport shard based on the current throw angle and facing direction.
	const UWorld* World = GetWorld();
	const AActor* OwnerActor = GetOwner();

	// If any of the required components are missing, return false to indicate that the launch cannot be computed.
	if (!World || !OwnerActor || !ProjectileClass)
	{
		return false;
	}

	// Direction from the hard-coded angle, mirrored by facing, and the plane normal for the shard to bounce off of.
	const FVector Forward = GetThrowForwardDirection();
	const float AngleRad = FMath::DegreesToRadians(GetCurrentThrowAngle());
	const FVector ThrowDir = (Forward * FMath::Cos(AngleRad) + FVector::UpVector * FMath::Sin(AngleRad)).GetSafeNormal();
	OutPlaneNormal = FVector::CrossProduct(Forward, FVector::UpVector).GetSafeNormal();

	// Spawn point; sweep from the character's centre so we never spawn inside a wall.
	const FVector Center = OwnerActor->GetActorLocation();
	OutSpawnLocation = Center + Forward * SpawnForwardOffset + FVector::UpVector * SpawnUpOffset;

	// Sweep to avoid spawning inside a wall. If the sweep hits something, spawn at the hit location instead.
	const ACPP_TeleportShard* ProjectileCDO = ProjectileClass->GetDefaultObject<ACPP_TeleportShard>();
	const float ProjectileRadius = ProjectileCDO ? FMath::Max(ProjectileCDO->GetCollisionRadius(), 1.f) : 8.f;

	// Sweep from the character's center to the desired spawn location, using a sphere with the projectile's radius. If it hits something, adjust the spawn location to the hit point.
	FCollisionQueryParams Params(SCENE_QUERY_STAT(TeleportThrowSpawn), false, OwnerActor);
	FHitResult SpawnHit;
	if (World->SweepSingleByChannel(SpawnHit, Center, OutSpawnLocation, FQuat::Identity, ECC_WorldDynamic,
		FCollisionShape::MakeSphere(ProjectileRadius), Params))
	{
		OutSpawnLocation = SpawnHit.Location;
	}

	// Compute the launch velocity based on the throw direction and speed. If bAddOwnerVelocity is true, add the owner's current velocity to the launch velocity.
	OutVelocity = ThrowDir * ThrowSpeed;
	if (bAddOwnerVelocity)
	{
		OutVelocity += OwnerActor->GetVelocity();
	}
	return true;
}


// ------------------------------------------------------------------ Throwing

void UTeleportAbilityComponent::ThrowProjectile()
{
	// Only throw the projectile if the ability is currently aiming. If it is in any other state, do nothing.
	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	GetWorld()->GetTimerManager().ClearTimer(AimCycleTimer);
	StopPreview();

	// If the launch cannot be computed (e.g. missing components), reset to Ready state and do nothing.
	FVector SpawnLocation, LaunchVelocity, PlaneNormal;
	if (!World || !OwnerActor || !ComputeLaunch(SpawnLocation, LaunchVelocity, PlaneNormal))
	{
		SetState(ETeleportAbilityState::Ready);
		return;
	}

	// Spawn the projectile using deferred spawning, so we can set up its properties before it is fully spawned. If the spawn fails, reset to Ready state and do nothing.
	const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
	ACPP_TeleportShard* Projectile = World->SpawnActorDeferred<ACPP_TeleportShard>(
		ProjectileClass, SpawnTransform, OwnerActor, Cast<APawn>(OwnerActor),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!Projectile)
	{
		SetState(ETeleportAbilityState::Ready);
		return;
	}

	// Set up the projectile's launch velocity and plane normal, and bind to its events for when it is armed, destroyed, or hits an enemy. Store a reference to the active projectile.
	Projectile->Launch(LaunchVelocity, PlaneNormal);
	Projectile->OnArmed.AddDynamic(this, &UTeleportAbilityComponent::HandleProjectileArmed);
	Projectile->OnDestroyed.AddDynamic(this, &UTeleportAbilityComponent::HandleProjectileDestroyed);
	Projectile->OnHitEnemy.AddDynamic(this, &UTeleportAbilityComponent::HandleShardHitEnemy);
	ActiveProjectile = Projectile;

	SetState(ETeleportAbilityState::InFlight);
	Projectile->FinishSpawning(SpawnTransform);

	OnThrown.Broadcast(AimIndex, Projectile);

	// Play the throw sound after spawning the shard
	if (Projectile->GetThrowSound())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), Projectile->GetThrowSound());
	}
}


// ------------------------------------------------------------------ Trajectory preview

void UTeleportAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	// Only tick while aiming and showing the trajectory preview. If not, stop ticking and clear the preview.
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (State != ETeleportAbilityState::Aiming || !bShowTrajectoryPreview)
	{
		StopPreview();
		return;
	}

	// Update the trajectory preview at a fixed rate, based on PreviewUpdatesPerSecond. If enough time has passed since the last update, recalculates and redraws the preview.
	TimeSincePreviewUpdate += DeltaTime;
	TimeSincePreviewRecalc += DeltaTime;
	if (TimeSincePreviewUpdate >= 1.f / FMath::Max(PreviewUpdatesPerSecond, 1.f))
	{
		TimeSincePreviewUpdate = 0.f;
		UpdateTrajectoryPreview(false);
	}
}


// ------------------------------------------------------------------ Trajectory preview helpers

void UTeleportAbilityComponent::StopPreview()
{
	// Stop ticking and clear the trajectory preview. This is called when the player stops aiming or when the component is destroyed.
	SetComponentTickEnabled(false);
	ClearTrajectoryPreview();
}

void UTeleportAbilityComponent::ClearTrajectoryPreview()
{
	// Clear the trajectory preview by removing all instances from the PreviewSprites component
	// and hiding the PreviewLandingMarker component.
	// Also invalidate the cached launch position and velocity so that the next aim recalculates the trajectory.

	if (PreviewSprites && PreviewSprites->GetInstanceCount() > 0)
	{
		PreviewSprites->ClearInstances();
	}
	if (PreviewLandingMarker)
	{
		PreviewLandingMarker->SetVisibility(false);
	}

	// Invalidate the cache so the next aim always recalculates.
	LastPreviewStart = FVector(FLT_MAX);
	LastPreviewVelocity = FVector(FLT_MAX);
}

void UTeleportAbilityComponent::UpdateTrajectoryPreview(bool bForce)
{
	// Update the trajectory preview by simulating the projectile's flight and placing dots along its path.
	// If bForce is true, the preview is recalculated regardless of whether the launch parameters have changed.

	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	if (!World || !OwnerActor || !ProjectileClass)
	{
		return;
	}

	FVector Position, Velocity, PlaneNormal;
	if (!ComputeLaunch(Position, Velocity, PlaneNormal))
	{
		return;
	}

	// If the launch parameters haven't changed since the last preview, and not enough time has passed, skip the recalculation to save performance.
	const bool bUnchanged = Position.Equals(LastPreviewStart, 0.5f) && Velocity.Equals(LastPreviewVelocity, 0.5f);
	if (!bForce && bUnchanged && TimeSincePreviewRecalc < 0.25f)
	{
		return;
	}
	LastPreviewStart = Position;
	LastPreviewVelocity = Velocity;
	TimeSincePreviewRecalc = 0.f;

	// Get the default object of the projectile class to access its properties, such as gravity scale, collision radius, and bounce multiplier.
	const ACPP_TeleportShard* CDO = ProjectileClass->GetDefaultObject<ACPP_TeleportShard>();
	if (!CDO)
	{
		return;
	}
	// Get the projectile's movement and collision components to access their properties.
	const UProjectileMovementComponent* CDOMovement = CDO->GetProjectileMovement();
	const USphereComponent* CDOCollision = CDO->GetCollisionComponent();

	// ---- Simulation parameters ----
	// Gravity is scaled by the projectile's gravity scale, and the floor threshold and bounce multiplier are taken from the projectile's properties.
	// The radius is clamped to a minimum of 1 to avoid issues with zero or negative radius.
	// The simulation is constrained to the 2D plane defined by the plane normal, and the sweep uses the projectile's collision channel and response settings.
	// The simulation runs in fixed time steps, placing dots along the path at regular spacing, and stops when the projectile lands or exceeds the maximum preview time or bounces.

	const float GravityScale = CDOMovement ? CDOMovement->ProjectileGravityScale : 1.f;
	const FVector Gravity(0.f, 0.f, World->GetGravityZ() * GravityScale);
	const float Radius = FMath::Max(CDO->GetCollisionRadius(), 1.f);
	const float FloorThreshold = CDO->GetFloorNormalThreshold();
	const float BounceMultiplier = CDO->GetWallBounceSpeedMultiplier();

	// Keep the simulation on the 2D plane, like the real shard's plane constraint.
	Velocity = FVector::VectorPlaneProject(Velocity, PlaneNormal);


	// Sweep parameters: use the projectile's collision channel and response settings, and ignore the owner actor to avoid self-collision.
	FCollisionQueryParams Params(SCENE_QUERY_STAT(TeleportPreview), false, OwnerActor);
	FCollisionResponseParams ResponseParams;
	if (CDOCollision)
	{
		ResponseParams.CollisionResponse = CDOCollision->GetCollisionResponseToChannels();
	}
	const ECollisionChannel Channel = CDOCollision ? CDOCollision->GetCollisionObjectType() : ECC_WorldDynamic;
	const FCollisionShape Shape = FCollisionShape::MakeSphere(Radius);

	// ---- Simulate the flight, placing dots as it goes ----
	const float TimeStep = FMath::Clamp(PreviewTimeStep, 0.008f, 0.1f);
	const float Spacing = FMath::Max(DotSpacing, 1.f);
	const int32 DotCap = FMath::Max(MaxPreviewDots, 2);

	TArray<FVector, TInlineAllocator<64>> DotPositions;
	DotPositions.Add(Position); // first dot at the hand
	float DistanceSinceLastDot = 0.f;

	auto AddSegmentDots = [&](const FVector& A, const FVector& B)
		{
			const float SegmentLength = FVector::Dist(A, B);
			float Along = 0.f;
			while (DotPositions.Num() < DotCap && DistanceSinceLastDot + (SegmentLength - Along) >= Spacing)
			{
				Along += Spacing - DistanceSinceLastDot;
				DotPositions.Add(FMath::Lerp(A, B, SegmentLength > 0.f ? Along / SegmentLength : 0.f));
				DistanceSinceLastDot = 0.f;
			}
			DistanceSinceLastDot += SegmentLength - Along;
		};

	bool bLanded = false;
	FVector LandingPoint = FVector::ZeroVector;
	int32 Bounces = 0;

	//-- Simulate the projectile's flight in fixed time steps, checking for collisions and placing dots along the path.
	// The simulation stops when the projectile lands, exceeds the maximum preview time, or reaches the maximum number of bounces.

	for (float Time = 0.f; Time < MaxPreviewTime && DotPositions.Num() < DotCap; Time += TimeStep)
	{
		const FVector Delta = Velocity * TimeStep + 0.5f * Gravity * TimeStep * TimeStep;
		Velocity += Gravity * TimeStep;
		const FVector Target = Position + Delta;

		FHitResult Hit;
		if (World->SweepSingleByChannel(Hit, Position, Target, FQuat::Identity, Channel, Shape, Params, ResponseParams))
		{
			AddSegmentDots(Position, Hit.Location);
			Position = Hit.Location;

			const FVector Normal = Hit.ImpactNormal;
			if (Normal.Z >= FloorThreshold)
			{
				// Floor: the real shard stops dead here.
				bLanded = true;
				LandingPoint = Position;
				break;
			}

			// Wall / ceiling: mirror the velocity, same as ACPP_TeleportShard::HandleBounce.
			if (++Bounces > MaxPreviewBounces)
			{
				break;
			}
			Velocity = FVector::VectorPlaneProject(Velocity.MirrorByVector(Normal) * BounceMultiplier, PlaneNormal);
			Position += Normal * 0.5f; // step off the surface so the next sweep doesn't start inside it
		}
		else
		{
			AddSegmentDots(Position, Target);
			Position = Target;
		}
	}

	// ---- Draw the dots and landing marker ----
	if (TrajectoryDotSprite)
	{
		if (!PreviewSprites)
		{
			PreviewSprites = NewObject<UPaperGroupedSpriteComponent>(OwnerActor, TEXT("TeleportTrajectoryPreview"));
			PreviewSprites->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			PreviewSprites->SetGenerateOverlapEvents(false);
			PreviewSprites->SetCastShadow(false);
			PreviewSprites->RegisterComponent();
			PreviewSprites->SetWorldTransform(FTransform::Identity);
		}

		// Reuse existing dots instead of deleting and recreating them every update:
		// add/remove only the difference, move the rest, and refresh the render data once.
		const int32 NumDots = DotPositions.Num();
		while (PreviewSprites->GetInstanceCount() > NumDots)
		{
			PreviewSprites->RemoveInstance(PreviewSprites->GetInstanceCount() - 1);
		}
		const int32 Existing = PreviewSprites->GetInstanceCount();

		//-- Update the existing dots and add new ones as needed, setting their transform and color based on their position along the trajectory.
		for (int32 i = 0; i < NumDots; ++i)
		{
			const float T = NumDots > 1 ? float(i) / float(NumDots - 1) : 0.f;
			FLinearColor Color = DotColor;
			Color.A = FMath::Lerp(DotColor.A, EndDotAlpha * DotColor.A, T);
			const FTransform DotTransform(FRotator::ZeroRotator, DotPositions[i] + PreviewOffset, FVector(DotScale));

			if (i < Existing)
			{
				PreviewSprites->UpdateInstanceTransform(i, DotTransform, true, false);
				PreviewSprites->UpdateInstanceColor(i, Color, false);
			}
			else
			{
				PreviewSprites->AddInstance(DotTransform, TrajectoryDotSprite, true, Color);
			}
		}
		PreviewSprites->MarkRenderStateDirty();

		// Landing marker: one separate sprite, just moved and shown/hidden.
		if (bLanded)
		{
			if (!PreviewLandingMarker)
			{
				PreviewLandingMarker = NewObject<UPaperSpriteComponent>(OwnerActor, TEXT("TeleportLandingMarker"));
				PreviewLandingMarker->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				PreviewLandingMarker->SetGenerateOverlapEvents(false);
				PreviewLandingMarker->SetCastShadow(false);
				PreviewLandingMarker->RegisterComponent();
			}
			PreviewLandingMarker->SetSprite(LandingMarkerSprite ? LandingMarkerSprite.Get() : TrajectoryDotSprite.Get());
			PreviewLandingMarker->SetSpriteColor(DotColor);
			PreviewLandingMarker->SetWorldLocationAndRotation(LandingPoint + PreviewOffset, FRotator::ZeroRotator);
			PreviewLandingMarker->SetWorldScale3D(FVector(LandingMarkerScale));
			PreviewLandingMarker->SetVisibility(true);
		}
		else if (PreviewLandingMarker)
		{
			PreviewLandingMarker->SetVisibility(false);
		}
	}

	//-- Debug drawing: if no dot sprite is assigned, draw debug points instead (editor/dev builds only).
#if ENABLE_DRAW_DEBUG
	else
	{
		// No dot sprite assigned yet: temporary debug dots (editor/dev builds only).
		// Debug points expire, so always redraw them, and keep each set alive until the next update.
		LastPreviewStart = FVector(FLT_MAX);
		const float LifeTime = 1.1f / FMath::Max(PreviewUpdatesPerSecond, 1.f);
		const float DebugSize = FMath::Max(Radius * 0.5f, 4.f);
		for (const FVector& Dot : DotPositions)
		{
			DrawDebugPoint(World, Dot + PreviewOffset, DebugSize * DotScale, DotColor.ToFColor(true), false, LifeTime, SDPG_Foreground);
		}
		if (bLanded)
		{
			DrawDebugPoint(World, LandingPoint + PreviewOffset, DebugSize * LandingMarkerScale, FColor::Cyan, false, LifeTime, SDPG_Foreground);
		}
	}
#endif
}


// ------------------------------------------------------------------ Projectile events

void UTeleportAbilityComponent::HandleProjectileArmed(ACPP_TeleportShard* Projectile)
{
	// If the armed projectile is the active one and the ability is in flight, transition to the Armed state and broadcast the OnTeleportArmed event.
	if (Projectile == ActiveProjectile.Get() && State == ETeleportAbilityState::InFlight)
	{
		SetState(ETeleportAbilityState::Armed);
		OnTeleportArmed.Broadcast();
	}
}

void UTeleportAbilityComponent::HandleProjectileDestroyed(AActor* DestroyedActor)
{
	// If the destroyed projectile is the active one, reset the reference and handle the fizzle event if the ability was in flight or armed.
	// If the ability is currently teleporting, do nothing as it was destroyed on purpose by TryTeleport.
	if (bIsTeleporting)
	{
		return; // destroyed on purpose by TryTeleport
	}

	ActiveProjectile.Reset();

	if (State == ETeleportAbilityState::InFlight || State == ETeleportAbilityState::Armed)
	{
		OnProjectileFizzled.Broadcast();
		if (bCooldownOnFizzle)
		{
			StartCooldown();
		}
		else
		{
			SetState(ETeleportAbilityState::Ready);
		}
	}
}

void UTeleportAbilityComponent::TryTeleport()
{
	// Only teleport if the ability is armed and the projectile is valid. If not, do nothing.
	ACPP_TeleportShard* Projectile = ActiveProjectile.Get();
	AActor* OwnerActor = GetOwner();
	if (!Projectile || !Projectile->IsArmed() || !OwnerActor)
	{
		return;
	}

	// Teleport the owner actor to the projectile's location, adjusting for the capsule size if it has landed on a floor.
	// If the teleport fails (e.g. no room), broadcast the OnTeleportFailed event and do nothing.
	const FVector From = OwnerActor->GetActorLocation();
	const FVector VelocityBefore = OwnerActor->GetVelocity();
	FVector Destination = Projectile->GetActorLocation();

	// If resting on a floor, lift the character so its capsule sits on top of it.
	if (Projectile->HasLanded())
	{
		float Radius = 0.f, HalfHeight = 0.f;
		OwnerActor->GetSimpleCollisionCylinder(Radius, HalfHeight);
		const FVector N = Projectile->GetLandingNormal();
		const float Offset = FMath::Lerp(Radius, HalfHeight, FMath::Abs(N.Z)) + TeleportSurfacePadding;
		Destination += N * Offset;
	}

	// bNoCheck = false: the engine nudges us out of walls if the spot is tight,
	// and returns false if there is no room at all.
	if (!OwnerActor->TeleportTo(Destination, OwnerActor->GetActorRotation(), false, false))
	{
		OnTeleportFailed.Broadcast();
		return;
	}

	// Keep part of the speed the player had before teleporting.
	if (UMovementComponent* MoveComp = OwnerActor->FindComponentByClass<UMovementComponent>())
	{
		const FVector Kept(
			VelocityBefore.X * HorizontalMomentumKept,
			VelocityBefore.Y * HorizontalMomentumKept,
			VelocityBefore.Z * VerticalMomentumKept);

		if (Kept.IsNearlyZero())
		{
			MoveComp->StopMovementImmediately();
		}
		else
		{
			MoveComp->Velocity = Kept;
			MoveComp->UpdateComponentVelocity();
		}
	}

	// Play the teleport sound and spawn particles at both positions before destroying the shard.
	if (Projectile->GetTeleportSound())
	{
		UGameplayStatics::PlaySound2D(GetWorld(), Projectile->GetTeleportSound());
	}
	if (UNiagaraSystem* TeleportFX = Projectile->GetTeleportParticles())
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), TeleportFX, From);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), TeleportFX, OwnerActor->GetActorLocation());
	}

	// Destroy the projectile and reset the active reference, then broadcast the OnTeleported event and start the cooldown. 
	// If hit-stop on arrival is enabled, start the hit-stop effect.

	bIsTeleporting = true;
	Projectile->OnDestroyed.RemoveAll(this);
	Projectile->OnArmed.RemoveAll(this);
	Projectile->OnHitEnemy.RemoveAll(this);
	Projectile->Destroy();
	ActiveProjectile.Reset();
	bIsTeleporting = false;

	OnTeleported.Broadcast(From, OwnerActor->GetActorLocation());
	StartCooldown();

	if (bHitStopOnArrival)
	{
		StartHitStop();
	}
}


// ------------------------------------------------------------------ Hit-stop

void UTeleportAbilityComponent::StartHitStop()
{
	// Only start hit-stop if the duration is positive and the world is valid.
	// If hit-stop is already active, save the current time dilation to restore it later.
	UWorld* World = GetWorld();
	if (!World || HitStopDuration <= 0.f)
	{
		return;
	}

	if (!bHitStopActive)
	{
		SavedTimeDilation = UGameplayStatics::GetGlobalTimeDilation(World);
	}
	bHitStopActive = true;

	const float Dilation = FMath::Clamp(HitStopTimeDilation, 0.01f, 1.f);
	UGameplayStatics::SetGlobalTimeDilation(World, Dilation);

	// Timers run on (slowed) game time, so scale the timer down to get the real-time duration.
	World->GetTimerManager().SetTimer(HitStopTimer, this, &UTeleportAbilityComponent::EndHitStop,
		FMath::Max(HitStopDuration * Dilation, 0.0001f), false);
}

void UTeleportAbilityComponent::EndHitStop()
{
	// Only end hit-stop if it is currently active. If not, do nothing.
	if (!bHitStopActive)
	{
		return;
	}
	bHitStopActive = false;

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(HitStopTimer);
		UGameplayStatics::SetGlobalTimeDilation(World, SavedTimeDilation);
	}
}


// ------------------------------------------------------------------ Enemies

void UTeleportAbilityComponent::HandleShardHitEnemy(ACPP_TeleportShard* Projectile, AActor* Enemy)
{
	// Only apply damage and stun if the projectile is the active one.
	// If not, do nothing, as the projectile may have been destroyed or replaced by another one.
	if (!Enemy)
	{
		return;
	}

	if (EnemyDamage > 0.f)
	{
		const APawn* OwnerPawn = Cast<APawn>(GetOwner());
		UGameplayStatics::ApplyDamage(Enemy, EnemyDamage, OwnerPawn ? OwnerPawn->GetController() : nullptr, Projectile, nullptr);
	}

	if (EnemyStunDuration > 0.f)
	{
		StunEnemy(Enemy, EnemyStunDuration);
	}

	OnShardHitEnemy.Broadcast(Enemy);
}

void UTeleportAbilityComponent::StunEnemy(AActor* Enemy, float Duration)
{
	// Only stun the enemy if the duration is positive and the world is valid.
	// If the enemy is already stunned, restart the timer.
	UWorld* World = GetWorld();
	if (!World || !Enemy)
	{
		return;
	}

	TWeakObjectPtr<AActor> WeakEnemy(Enemy);
	FTeleportStunInfo* Info = StunnedEnemies.Find(WeakEnemy);
	if (!Info)
	{
		Info = &StunnedEnemies.Add(WeakEnemy);
		Info->OriginalTimeDilation = Enemy->CustomTimeDilation;
	}

	// Freeze the enemy: its movement, animation and Blueprint tick all run at 0 speed.
	Enemy->CustomTimeDilation = 0.f;

	// Re-hitting a stunned enemy restarts the timer.
	World->GetTimerManager().SetTimer(Info->Timer,
		FTimerDelegate::CreateUObject(this, &UTeleportAbilityComponent::UnstunEnemy, WeakEnemy),
		Duration, false);
}

void UTeleportAbilityComponent::UnstunEnemy(TWeakObjectPtr<AActor> Enemy)
{
	// Only unstun the enemy if it is still valid and was previously stunned. If not, do nothing.
	FTeleportStunInfo Info;
	if (!StunnedEnemies.RemoveAndCopyValue(Enemy, Info))
	{
		return;
	}
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(Info.Timer);
	}
	if (AActor* Actor = Enemy.Get())
	{
		Actor->CustomTimeDilation = Info.OriginalTimeDilation;
	}
}


// ------------------------------------------------------------------ Cooldown

void UTeleportAbilityComponent::StartCooldown()
{
	// Only start cooldown if the ability is not already in cooldown. If it is, do nothing.
	if (CooldownDuration <= 0.f)
	{
		FinishCooldown();
		return;
	}
	SetState(ETeleportAbilityState::Cooldown);
	GetWorld()->GetTimerManager().SetTimer(CooldownTimer, this, &UTeleportAbilityComponent::FinishCooldown, CooldownDuration, false);
}

void UTeleportAbilityComponent::FinishCooldown()
{
	// Only finish cooldown if the ability is currently in cooldown. If it is not, do nothing.
	SetState(ETeleportAbilityState::Ready);
	OnCooldownFinished.Broadcast();
}