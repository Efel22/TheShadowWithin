// CPP_TeleportShard.cpp

#include "CPP_TeleportShard.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ACPP_TeleportShard::ACPP_TeleportShard()
{
	PrimaryActorTick.bCanEverTick = false;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(8.f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionObjectType(ECC_WorldDynamic);
	Collision->SetCollisionResponseToAllChannels(ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);       // never hits / blocks characters
	Collision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	Collision->SetCanEverAffectNavigation(false);
	Collision->CanCharacterStepUpOn = ECB_No;
	RootComponent = Collision;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = Collision;
	ProjectileMovement->InitialSpeed = 0.f;               // velocity is supplied by Launch()
	ProjectileMovement->MaxSpeed = 0.f;                   // no cap
	ProjectileMovement->bInitialVelocityInLocalSpace = false;
	ProjectileMovement->bRotationFollowsVelocity = false; // keep the sprite facing the camera
	ProjectileMovement->ProjectileGravityScale = 1.f;
	ProjectileMovement->bShouldBounce = true;             // we override the result in HandleBounce
	ProjectileMovement->Bounciness = 1.f;
	ProjectileMovement->Friction = 0.f;
	ProjectileMovement->bForceSubStepping = true;         // more accurate bounces at high speed
	ProjectileMovement->MaxSimulationTimeStep = 0.016f;
}

void ACPP_TeleportShard::BeginPlay()
{
	Super::BeginPlay();

	ProjectileMovement->OnProjectileBounce.AddDynamic(this, &ACPP_TeleportShard::HandleBounce);
	ProjectileMovement->OnProjectileStop.AddDynamic(this, &ACPP_TeleportShard::HandleStop);

	if (UnarmedLifetime > 0.f)
	{
		SetLifeSpan(UnarmedLifetime);
	}
}

void ACPP_TeleportShard::Launch(const FVector& LaunchVelocity, const FVector& PlaneNormal)
{
	if (AActor* OwnerActor = GetOwner())
	{
		Collision->IgnoreActorWhenMoving(OwnerActor, true);
	}

	// Lock to the 2D gameplay plane so bounces never push it toward/away from the camera.
	if (!PlaneNormal.IsNearlyZero())
	{
		ProjectileMovement->SetPlaneConstraintEnabled(true);
		ProjectileMovement->SetPlaneConstraintNormal(PlaneNormal.GetSafeNormal());
		ProjectileMovement->SetPlaneConstraintOrigin(GetActorLocation());
	}

	ProjectileMovement->Velocity = LaunchVelocity;
}

float ACPP_TeleportShard::GetCollisionRadius() const
{
	return Collision ? Collision->GetScaledSphereRadius() : 0.f;
}

void ACPP_TeleportShard::HandleBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	const FVector Normal = ImpactResult.ImpactNormal;

	if (Normal.Z >= FloorNormalThreshold)
	{
		// Floor: zero the velocity. The movement component then stops simulating
		// on its own and fires OnProjectileStop -> HandleStop (lands + arms).
		ProjectileMovement->Velocity = FVector::ZeroVector;
		return;
	}

	// Wall or ceiling: mirror the incoming velocity so the outgoing angle
	// matches the angle it was thrown at, then scale the speed down.
	ProjectileMovement->Velocity = ImpactVelocity.MirrorByVector(Normal) * WallBounceSpeedMultiplier;

	Arm();
	BP_OnImpact(ImpactResult, true);
	OnImpact.Broadcast(ImpactResult, true);
}

void ACPP_TeleportShard::HandleStop(const FHitResult& ImpactResult)
{
	if (bLanded)
	{
		return;
	}
	bLanded = true;
	LandingNormal = ImpactResult.bBlockingHit ? FVector(ImpactResult.ImpactNormal) : FVector::UpVector;

	if (bAttachToMovingSurfaces)
	{
		if (USceneComponent* HitComp = ImpactResult.GetComponent())
		{
			if (HitComp->Mobility == EComponentMobility::Movable)
			{
				AttachToComponent(HitComp, FAttachmentTransformRules::KeepWorldTransform);
			}
		}
	}

	Arm();
	BP_OnImpact(ImpactResult, false);
	OnImpact.Broadcast(ImpactResult, false);
}

void ACPP_TeleportShard::Arm()
{
	if (bArmed)
	{
		return;
	}
	bArmed = true;

	// Replace the "never touched anything" lifespan with the armed lifespan (0 clears it).
	SetLifeSpan(ArmedLifetime);

	BP_OnArmed();
	OnArmed.Broadcast(this);
}