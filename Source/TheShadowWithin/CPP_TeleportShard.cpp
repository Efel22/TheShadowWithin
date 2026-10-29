// CPP_TeleportShard.cpp

#include "CPP_TeleportShard.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
ACPP_TeleportShard::ACPP_TeleportShard()
{
	// Set this actor to call Tick() every frame.
	// You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// detects enemies without ever blocking anyone
	//simply creating a sphere collision component and setting it to overlap pawns, while blocking everything else
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	Collision->InitSphereRadius(8.f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionObjectType(ECC_WorldDynamic);
	Collision->SetCollisionResponseToAllChannels(ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);      
	Collision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Collision->SetCollisionResponseToChannel(ECC_Visibility, ECR_Ignore);
	Collision->SetCanEverAffectNavigation(false);
	Collision->CanCharacterStepUpOn = ECB_No;
	Collision->SetGenerateOverlapEvents(true);
	RootComponent = Collision;

	// projectile movement component handles the physics simulation and bounces
	// we override the bounce result in HandleBounce to control the speed and angle of the bounce
	// we also use HandleStop to detect when it has landed on a floor
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = Collision;
	ProjectileMovement->InitialSpeed = 0.f;               
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


// Called when the game starts or when spawned, or when BeginPlay is called after spawning with FinishSpawning
// This is where we bind the bounce and stop events, and set up the collision overlap for enemies if needed.
void ACPP_TeleportShard::BeginPlay()
{
	Super::BeginPlay();

	ProjectileMovement->OnProjectileBounce.AddDynamic(this, &ACPP_TeleportShard::HandleBounce);
	ProjectileMovement->OnProjectileStop.AddDynamic(this, &ACPP_TeleportShard::HandleStop);

	// If we want to interact with enemies, bind the overlap event. Otherwise, ignore pawns.
	if (bInteractWithEnemies)
	{
		Collision->OnComponentBeginOverlap.AddDynamic(this, &ACPP_TeleportShard::HandleBeginOverlap);
	}
	else
	{
		Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}

	if (UnarmedLifetime > 0.f)
	{
		SetLifeSpan(UnarmedLifetime);
	}
}

// Launch the shard with a given velocity and optional plane normal to constrain movement to a 2D plane.
void ACPP_TeleportShard::Launch(const FVector& LaunchVelocity, const FVector& PlaneNormal)
{
	// Ignore the owner so it doesn't collide with the player who threw it.
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
		PlaneNormalUsed = PlaneNormal.GetSafeNormal();
	}

	ProjectileMovement->Velocity = LaunchVelocity;
}

// Get the collision radius of the shard, which is used for overlap checks and bounces.
float ACPP_TeleportShard::GetCollisionRadius() const
{
	return Collision ? Collision->GetScaledSphereRadius() : 0.f;
}

// Handle a bounce off a surface. If it's a floor, stop the shard. If it's a wall or ceiling, reflect the velocity and scale it down.
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

// Handle the shard coming to rest on a floor. This is where we mark it as landed, arm it, and optionally attach it to a moving surface.
void ACPP_TeleportShard::HandleStop(const FHitResult& ImpactResult)
{
	if (bLanded)
	{
		return;
	}
	bLanded = true;
	LandingNormal = ImpactResult.bBlockingHit ? FVector(ImpactResult.ImpactNormal) : FVector::UpVector;

	// Resting on the floor: enemies walking over it shouldn't trigger hits.
	Collision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	// If the shard landed on a moving platform, attach to it so it moves with the platform.
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

	// Arm the shard so the player can teleport to it.
	Arm();
	BP_OnImpact(ImpactResult, false);
	OnImpact.Broadcast(ImpactResult, false);
}

// Arm the shard so it can be used for teleportation. This sets the armed flag, updates the lifespan, and triggers events.
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

// Handle the shard overlapping with an enemy. This is where we bounce off the enemy, arm the shard if needed, and trigger different events. We ignore the owner, instigator, and any already-hit enemies.
void ACPP_TeleportShard::HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this || OtherActor == GetOwner() || OtherActor == GetInstigator() || bLanded)
	{
		return;
	}
	// Only pawns are considered enemies.
	if (!EnemyTag.IsNone() && !OtherActor->ActorHasTag(EnemyTag))
	{
		return;
	}
	if (HitEnemies.Contains(OtherActor))
	{
		return;
	}
	HitEnemies.Add(OtherActor);

	// Bounce away from the enemy's centre (kept on the 2D plane).
	FVector Velocity = ProjectileMovement->Velocity;
	FVector Normal = GetActorLocation() - OtherActor->GetActorLocation();
	if (!PlaneNormalUsed.IsNearlyZero())
	{
		Normal = FVector::VectorPlaneProject(Normal, PlaneNormalUsed);
	}
	if (!Normal.Normalize())
	{
		Normal = -Velocity.GetSafeNormal();
	}
	if (FVector::DotProduct(Velocity, Normal) < 0.f)
	{
		Velocity = Velocity.MirrorByVector(Normal);
	}
	ProjectileMovement->Velocity = Velocity * EnemyBounceSpeedMultiplier;

	if (bArmOnEnemyHit)
	{
		Arm();
	}

	BP_OnHitEnemy(OtherActor);
	OnHitEnemy.Broadcast(this, OtherActor);
}