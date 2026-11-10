
#include "MyAssets/Actors/CPP_FallingPlatform.h"

#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "PaperTileMapComponent.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

#include "MyAssets/Characters/CPP_PlayerChar.h"

// *******************************************************************************
//                                  CONSTRUCTOR
// *******************************************************************************

// Sets default values
ACPP_FallingPlatform::ACPP_FallingPlatform()
{
	// No Tick required for this actor
	PrimaryActorTick.bCanEverTick = false;

	// ROOT
	DefaultSceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));
	RootComponent = DefaultSceneRoot;

	// ***********************************************************************
	//                  PLATFORM DEACTIVATION TRIGGER BOX
	// ***********************************************************************

	PlatformDeactivationTriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Platform Deactivation Trigger Box"));
	PlatformDeactivationTriggerBox->SetupAttachment(RootComponent);

	// Detect the player without physically blocking them
	PlatformDeactivationTriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PlatformDeactivationTriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	PlatformDeactivationTriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PlatformDeactivationTriggerBox->SetGenerateOverlapEvents(true);

	// Default collision size (Y thickness = 50.f)
	PlatformDeactivationTriggerBox->SetBoxExtent(FVector(200.0f, 50.0f, 100.0f));

	// ***********************************************************************
	//                              PLATFORM HITBOX
	// ***********************************************************************

	PlatformHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("Platform Hitbox"));
	PlatformHitbox->SetupAttachment(RootComponent);

	// Default collision size (Y thickness = 25.f)
	PlatformHitbox->SetBoxExtent(FVector(200.0f, 25.0f, 25.0f));

	// Make the bridge block the player and other physical objects
	PlatformHitbox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	PlatformHitbox->SetCollisionObjectType(ECC_WorldDynamic);
	PlatformHitbox->SetCollisionResponseToAllChannels(ECR_Ignore);
	PlatformHitbox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	PlatformHitbox->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	PlatformHitbox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

	PlatformHitbox->SetGenerateOverlapEvents(false);

	// Start stationary and enable gravity when triggered
	PlatformHitbox->SetSimulatePhysics(false);
	PlatformHitbox->SetEnableGravity(true);

	// Generate events when the platform hits another surface
	PlatformHitbox->SetNotifyRigidBodyCollision(true);

	// ***********************************************************************
	//                              PLATFORM TILEMAP
	// ***********************************************************************

	PlatformTileMap = CreateDefaultSubobject<UPaperTileMapComponent>(TEXT("Platform TileMap"));

	// Attach the tilemap to the physical hitbox
	PlatformTileMap->SetupAttachment(PlatformHitbox);

	// The tilemap is visual only; the Platform Hitbox handles collision
	PlatformTileMap->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlatformTileMap->SetGenerateOverlapEvents(false);

	// ***********************************************************************
	//                              FALLING PARTICLES
	// ***********************************************************************

	FallingParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Falling Particles"));

	// Attach the particles to the physical hitbox
	FallingParticles->SetupAttachment(PlatformHitbox);

	// Do not emit particles until the bridge begins falling
	FallingParticles->SetAutoActivate(false);

	// ***********************************************************************
	//                              LANDING PARTICLES
	// ***********************************************************************

	LandingParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Landing Particles"));

	// Attach to the root so the effect can remain at the landing location
	LandingParticles->SetupAttachment(RootComponent);

	// Only activate after the platform has landed
	LandingParticles->SetAutoActivate(false);

	// The actor itself handles destruction after the effect finishes
	LandingParticles->SetAutoDestroy(false);
}

// *******************************************************************************
//                                   BEGIN PLAY
// *******************************************************************************

// Called when the game starts or when spawned
void ACPP_FallingPlatform::BeginPlay()
{
	Super::BeginPlay();

	// Listen for the player entering the trigger
	PlatformDeactivationTriggerBox->OnComponentBeginOverlap.AddDynamic(
		this,
		&ACPP_FallingPlatform::OnTriggerBeginOverlap
	);

	// Listen for the platform colliding with the ground
	PlatformHitbox->OnComponentHit.AddDynamic(
		this,
		&ACPP_FallingPlatform::OnPlatformHit
	);

	// Listen for the landing Niagara system finishing
	LandingParticles->OnSystemFinished.AddDynamic(
		this,
		&ACPP_FallingPlatform::OnLandingParticlesFinished
	);

	// Make sure both particle components start disabled
	FallingParticles->Deactivate();
	LandingParticles->Deactivate();
}

// *******************************************************************************
//                             TRIGGER BEGIN OVERLAP
// *******************************************************************************

void ACPP_FallingPlatform::OnTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	// Prevent the platform from activating multiple times
	if (bHasBeenTriggered)
	{
		return;
	}

	// Only the player can activate the platform
	ACPP_PlayerChar* Player = Cast<ACPP_PlayerChar>(OtherActor);

	if (!Player || Player->IsDead())
	{
		return;
	}

	bHasBeenTriggered = true;

	// Play Sound
	if (Sound_Triggered)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			Sound_Triggered,
			PlatformHitbox->GetComponentLocation()
		);
	}

	// Start the timer before the bridge begins falling
	if (FallDelay > 0.0f)
	{
		GetWorldTimerManager().SetTimer(
			FallTimer,
			this,
			&ACPP_FallingPlatform::StartFalling,
			FallDelay,
			false
		);
	}
	else
	{
		StartFalling();
	}
}

// *******************************************************************************
//                                 START FALLING
// *******************************************************************************

void ACPP_FallingPlatform::StartFalling()
{
	// Prevent the platform from falling multiple times
	if (bIsFalling || bHasLanded)
	{
		return;
	}

	bIsFalling = true;

	// Play Sound
	if (Sound_Falling)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			Sound_Falling,
			PlatformHitbox->GetComponentLocation()
		);
	}

	// Start emitting falling particles
	FallingParticles->Activate(true);

	// Enable physics so gravity makes the platform fall
	PlatformHitbox->SetSimulatePhysics(true);
	PlatformHitbox->WakeAllRigidBodies();
}

// *******************************************************************************
//                                 PLATFORM HIT
// *******************************************************************************

void ACPP_FallingPlatform::OnPlatformHit(
	UPrimitiveComponent* HitComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	FVector NormalImpulse,
	const FHitResult& Hit)
{
	// Ignore collisions before falling or after landing
	if (!bIsFalling || bHasLanded)
	{
		return;
	}

	if (!OtherActor || !OtherComp)
	{
		return;
	}

	// Ignore collisions with the player
	if (OtherActor->IsA(ACPP_PlayerChar::StaticClass()))
	{
		return;
	}

	// Only land when hitting a surface underneath the bridge
	if (Hit.ImpactNormal.Z < 0.5f)
	{
		return;
	}

	// Finish the falling process
	LandPlatform(Hit.ImpactPoint);
}

// *******************************************************************************
//                                 LAND PLATFORM
// *******************************************************************************

void ACPP_FallingPlatform::LandPlatform(const FVector& ImpactLocation)
{
	// Prevent the landing event from happening multiple times
	if (bHasLanded)
	{
		return;
	}

	bHasLanded = true;
	bIsFalling = false;

	// Stop the platform from moving
	PlatformHitbox->SetSimulatePhysics(false);

	// Stop the falling particles immediately
	FallingParticles->DeactivateImmediate();

	// Disable the bridge's physical collision
	PlatformHitbox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Disable the trigger to prevent additional overlaps
	PlatformDeactivationTriggerBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Hide the visual representation of the bridge
	PlatformTileMap->SetVisibility(false);

	// Play Sound
	if (Sound_Landed)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			Sound_Landed,
			ImpactLocation
		);
	}

	// Play landing particles at the collision location
	if (LandingParticles && LandingParticles->GetAsset())
	{
		LandingParticles->SetWorldLocation(ImpactLocation);
		LandingParticles->Activate(true);
	}
	else
	{
		// Destroy immediately if no landing system was assigned
		Destroy();
	}
}

// *******************************************************************************
//                          LANDING PARTICLES FINISHED
// *******************************************************************************

void ACPP_FallingPlatform::OnLandingParticlesFinished(UNiagaraComponent* FinishedComponent)
{
	// Make sure the completed effect is the landing effect
	if (FinishedComponent != LandingParticles || !bHasLanded)
	{
		return;
	}

	// Destroy the bridge after the landing particles finish
	Destroy();
}
