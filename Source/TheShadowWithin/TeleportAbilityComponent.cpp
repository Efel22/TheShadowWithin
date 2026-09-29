// TeleportAbilityComponent.cpp

#include "TeleportAbilityComponent.h"
#include "CPP_TeleportShard.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/MovementComponent.h"

UTeleportAbilityComponent::UTeleportAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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

FVector UTeleportAbilityComponent::GetThrowForwardDirection_Implementation() const
{
	FVector Forward = GetOwner() ? GetOwner()->GetActorForwardVector() : FVector::ForwardVector;
	Forward.Z = 0.f;
	if (!Forward.Normalize())
	{
		Forward = FVector::ForwardVector;
	}
	return Forward;
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
}

void UTeleportAbilityComponent::ThrowProjectile()
{
	UWorld* World = GetWorld();
	AActor* OwnerActor = GetOwner();
	GetWorld()->GetTimerManager().ClearTimer(AimCycleTimer);

	if (!World || !OwnerActor || !ProjectileClass)
	{
		SetState(ETeleportAbilityState::Ready);
		return;
	}

	// Direction from the hard-coded angle, mirrored by facing.
	const FVector Forward = GetThrowForwardDirection();
	const float AngleRad = FMath::DegreesToRadians(GetCurrentThrowAngle());
	const FVector ThrowDir = (Forward * FMath::Cos(AngleRad) + FVector::UpVector * FMath::Sin(AngleRad)).GetSafeNormal();
	const FVector PlaneNormal = FVector::CrossProduct(Forward, FVector::UpVector).GetSafeNormal();

	// Spawn point; sweep from the character's centre so we never spawn inside a wall.
	const FVector Center = OwnerActor->GetActorLocation();
	FVector SpawnLocation = Center + Forward * SpawnForwardOffset + FVector::UpVector * SpawnUpOffset;

	const ACPP_TeleportShard* ProjectileCDO = ProjectileClass->GetDefaultObject<ACPP_TeleportShard>();
	const float ProjectileRadius = ProjectileCDO ? FMath::Max(ProjectileCDO->GetCollisionRadius(), 1.f) : 8.f;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(TeleportThrowSpawn), false, OwnerActor);
	FHitResult SpawnHit;
	if (World->SweepSingleByChannel(SpawnHit, Center, SpawnLocation, FQuat::Identity, ECC_WorldDynamic,
		FCollisionShape::MakeSphere(ProjectileRadius), Params))
	{
		SpawnLocation = SpawnHit.Location;
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

	FVector LaunchVelocity = ThrowDir * ThrowSpeed;
	if (bAddOwnerVelocity)
	{
		LaunchVelocity += OwnerActor->GetVelocity();
	}

	Projectile->Launch(LaunchVelocity, PlaneNormal);
	Projectile->OnArmed.AddDynamic(this, &UTeleportAbilityComponent::HandleProjectileArmed);
	Projectile->OnDestroyed.AddDynamic(this, &UTeleportAbilityComponent::HandleProjectileDestroyed);
	ActiveProjectile = Projectile;

	SetState(ETeleportAbilityState::InFlight);
	Projectile->FinishSpawning(SpawnTransform);

	OnThrown.Broadcast(AimIndex, Projectile);
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