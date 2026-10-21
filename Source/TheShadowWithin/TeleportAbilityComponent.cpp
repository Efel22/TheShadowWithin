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

UTeleportAbilityComponent::UTeleportAbilityComponent()
{
	// Ticks ONLY while aiming, to redraw the trajectory preview every frame.
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	PrimaryComponentTick.TickGroup = TG_PostPhysics; // after the character has moved this frame
}

void UTeleportAbilityComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AimCycleTimer);
		World->GetTimerManager().ClearTimer(CooldownTimer);
	}

	if (ACPP_TeleportShard* Projectile = ActiveProjectile.Get())
	{
		Projectile->OnDestroyed.RemoveAll(this);
		Projectile->OnArmed.RemoveAll(this);
		Projectile->Destroy();
	}
	ActiveProjectile.Reset();

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

void UTeleportAbilityComponent::OnTeleportInputReleased()
{
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

void UTeleportAbilityComponent::CancelAim()
{
	if (State != ETeleportAbilityState::Aiming)
	{
		return;
	}
	GetWorld()->GetTimerManager().ClearTimer(AimCycleTimer);
	StopPreview();
	SetState(ETeleportAbilityState::Ready);
	OnAimCancelled.Broadcast();
}

// ------------------------------------------------------------------ Queries

float UTeleportAbilityComponent::GetCurrentThrowAngle() const
{
	return ThrowAnglesDegrees.IsValidIndex(AimIndex) ? ThrowAnglesDegrees[AimIndex] : 0.f;
}

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

void UTeleportAbilityComponent::SetFacingDirection(float Direction)
{
	// 0 = no input this frame: keep facing the last direction.
	if (!FMath::IsNearlyZero(Direction))
	{
		FacingSign = Direction > 0.f ? 1.f : -1.f;
		bHasExplicitFacing = true;
	}
}

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
	if (NewState == State)
	{
		return;
	}
	const ETeleportAbilityState OldState = State;
	State = NewState;
	OnStateChanged.Broadcast(NewState, OldState);
}

void UTeleportAbilityComponent::StartAiming()
{
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

	if (ThrowAnglesDegrees.Num() > 1)
	{
		GetWorld()->GetTimerManager().SetTimer(AimCycleTimer, this, &UTeleportAbilityComponent::AdvanceAim, AimCycleInterval, true);
	}

	if (bShowTrajectoryPreview)
	{
		SetComponentTickEnabled(true);
		TimeSincePreviewUpdate = 0.f;
		UpdateTrajectoryPreview(true); // show it immediately, not one frame later
	}
}

void UTeleportAbilityComponent::AdvanceAim()
{
	const int32 Num = ThrowAnglesDegrees.Num();
	if (Num <= 1)
	{
		return;
	}

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

	if (State == ETeleportAbilityState::Aiming && bShowTrajectoryPreview)
	{
		UpdateTrajectoryPreview(true); // new angle: update right away
	}
}

bool UTeleportAbilityComponent::ComputeLaunch(FVector& OutSpawnLocation, FVector& OutVelocity, FVector& OutPlaneNormal) const
{
	const UWorld* World = GetWorld();
	const AActor* OwnerActor = GetOwner();
	if (!World || !OwnerActor || !ProjectileClass)
	{
		return false;
	}

	// Direction from the hard-coded angle, mirrored by facing.
	const FVector Forward = GetThrowForwardDirection();
	const float AngleRad = FMath::DegreesToRadians(GetCurrentThrowAngle());
	const FVector ThrowDir = (Forward * FMath::Cos(AngleRad) + FVector::UpVector * FMath::Sin(AngleRad)).GetSafeNormal();
	OutPlaneNormal = FVector::CrossProduct(Forward, FVector::UpVector).GetSafeNormal();

	// Spawn point; sweep from the character's centre so we never spawn inside a wall.
	const FVector Center = OwnerActor->GetActorLocation();
	OutSpawnLocation = Center + Forward * SpawnForwardOffset + FVector::UpVector * SpawnUpOffset;

	const ACPP_TeleportShard* ProjectileCDO = ProjectileClass->GetDefaultObject<ACPP_TeleportShard>();
	const float ProjectileRadius = ProjectileCDO ? FMath::Max(ProjectileCDO->GetCollisionRadius(), 1.f) : 8.f;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(TeleportThrowSpawn), false, OwnerActor);
	FHitResult SpawnHit;
	if (World->SweepSingleByChannel(SpawnHit, Center, OutSpawnLocation, FQuat::Identity, ECC_WorldDynamic,
		FCollisionShape::MakeSphere(ProjectileRadius), Params))
	{
		OutSpawnLocation = SpawnHit.Location;
	}

	OutVelocity = ThrowDir * ThrowSpeed;
	if (bAddOwnerVelocity)
	{
		OutVelocity += OwnerActor->GetVelocity();
	}
	return true;
}

void UTeleportAbilityComponent::ThrowProjectile()
{
	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	GetWorld()->GetTimerManager().ClearTimer(AimCycleTimer);
	StopPreview();

	FVector SpawnLocation, LaunchVelocity, PlaneNormal;
	if (!World || !OwnerActor || !ComputeLaunch(SpawnLocation, LaunchVelocity, PlaneNormal))
	{
		SetState(ETeleportAbilityState::Ready);
		return;
	}

	const FTransform SpawnTransform(FRotator::ZeroRotator, SpawnLocation);
	ACPP_TeleportShard* Projectile = World->SpawnActorDeferred<ACPP_TeleportShard>(
		ProjectileClass, SpawnTransform, OwnerActor, Cast<APawn>(OwnerActor),
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!Projectile)
	{
		SetState(ETeleportAbilityState::Ready);
		return;
	}

	Projectile->Launch(LaunchVelocity, PlaneNormal);
	Projectile->OnArmed.AddDynamic(this, &UTeleportAbilityComponent::HandleProjectileArmed);
	Projectile->OnDestroyed.AddDynamic(this, &UTeleportAbilityComponent::HandleProjectileDestroyed);
	ActiveProjectile = Projectile;

	SetState(ETeleportAbilityState::InFlight);
	Projectile->FinishSpawning(SpawnTransform);

	OnThrown.Broadcast(AimIndex, Projectile);
}

// ------------------------------------------------------------------ Trajectory preview

void UTeleportAbilityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (State != ETeleportAbilityState::Aiming || !bShowTrajectoryPreview)
	{
		StopPreview();
		return;
	}

	// Throttle: recalculate at most PreviewUpdatesPerSecond times per second.
	TimeSincePreviewUpdate += DeltaTime;
	TimeSincePreviewRecalc += DeltaTime;
	if (TimeSincePreviewUpdate >= 1.f / FMath::Max(PreviewUpdatesPerSecond, 1.f))
	{
		TimeSincePreviewUpdate = 0.f;
		UpdateTrajectoryPreview(false);
	}
}

void UTeleportAbilityComponent::StopPreview()
{
	SetComponentTickEnabled(false);
	ClearTrajectoryPreview();
}

void UTeleportAbilityComponent::ClearTrajectoryPreview()
{
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

	// ---- Cache: if the throw hasn't changed, the arc hasn't either. Skip all the work. ----
	// (Still refresh a few times per second in case something moved into the path.)
	const bool bUnchanged = Position.Equals(LastPreviewStart, 0.5f) && Velocity.Equals(LastPreviewVelocity, 0.5f);
	if (!bForce && bUnchanged && TimeSincePreviewRecalc < 0.25f)
	{
		return;
	}
	LastPreviewStart = Position;
	LastPreviewVelocity = Velocity;
	TimeSincePreviewRecalc = 0.f;

	// Copy the real shard's physics + collision settings from its class defaults,
	// so the preview bounces and lands exactly where the real one will.
	const ACPP_TeleportShard* CDO = ProjectileClass->GetDefaultObject<ACPP_TeleportShard>();
	if (!CDO)
	{
		return;
	}
	const UProjectileMovementComponent* CDOMovement = CDO->GetProjectileMovement();
	const USphereComponent* CDOCollision = CDO->GetCollisionComponent();

	const float GravityScale = CDOMovement ? CDOMovement->ProjectileGravityScale : 1.f;
	const FVector Gravity(0.f, 0.f, World->GetGravityZ() * GravityScale);
	const float Radius = FMath::Max(CDO->GetCollisionRadius(), 1.f);
	const float FloorThreshold = CDO->GetFloorNormalThreshold();
	const float BounceMultiplier = CDO->GetWallBounceSpeedMultiplier();

	// Keep the simulation on the 2D plane, like the real shard's plane constraint.
	Velocity = FVector::VectorPlaneProject(Velocity, PlaneNormal);

	FCollisionQueryParams Params(SCENE_QUERY_STAT(TeleportPreview), false, OwnerActor);
	FCollisionResponseParams ResponseParams;
	if (CDOCollision)
	{
		ResponseParams.CollisionResponse = CDOCollision->GetCollisionResponseToChannels();
	}
	const ECollisionChannel Channel = CDOCollision ? CDOCollision->GetCollisionObjectType() : ECC_WorldDynamic;
	const FCollisionShape Shape = FCollisionShape::MakeSphere(Radius);

	// ---- Simulate the flight, placing dots as we go ----
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

	// ---- Draw ----
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

void UTeleportAbilityComponent::HandleProjectileArmed(ACPP_TeleportShard* Projectile)
{
	if (Projectile == ActiveProjectile.Get() && State == ETeleportAbilityState::InFlight)
	{
		SetState(ETeleportAbilityState::Armed);
		OnTeleportArmed.Broadcast();
	}
}

void UTeleportAbilityComponent::HandleProjectileDestroyed(AActor* DestroyedActor)
{
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
	ACPP_TeleportShard* Projectile = ActiveProjectile.Get();
	AActor* OwnerActor = GetOwner();
	if (!Projectile || !Projectile->IsArmed() || !OwnerActor)
	{
		return;
	}

	const FVector From = OwnerActor->GetActorLocation();
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

	if (bResetVelocityOnTeleport)
	{
		if (UMovementComponent* MoveComp = OwnerActor->FindComponentByClass<UMovementComponent>())
		{
			MoveComp->StopMovementImmediately();
		}
	}

	bIsTeleporting = true;
	Projectile->OnDestroyed.RemoveAll(this);
	Projectile->OnArmed.RemoveAll(this);
	Projectile->Destroy();
	ActiveProjectile.Reset();
	bIsTeleporting = false;

	OnTeleported.Broadcast(From, OwnerActor->GetActorLocation());
	StartCooldown();
}

void UTeleportAbilityComponent::StartCooldown()
{
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
	SetState(ETeleportAbilityState::Ready);
	OnCooldownFinished.Broadcast();
}