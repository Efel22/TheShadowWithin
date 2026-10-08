
#include "MyAssets/Actors/CPP_JumpingMushroom.h"

#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "PaperFlipbookComponent.h"
#include "PaperFlipbook.h"
#include "MyAssets/Characters/CPP_PlayerChar.h"
#include "NiagaraFunctionLibrary.h" // Used to spawn Niagara particles
#include "NiagaraSystem.h"
#include "Kismet/GameplayStatics.h" // Used to play sounds
#include "Sound/SoundBase.h"
#include "GameFramework/CharacterMovementComponent.h"

// *******************************************************************************
//                                  CONSTRUCTOR
// *******************************************************************************
ACPP_JumpingMushroom::ACPP_JumpingMushroom()
{
	// Disable Tick since the mushroom doesn't need it
	PrimaryActorTick.bCanEverTick = false;

	// Create the default root component
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// Create the mushroom's Flipbook Component
	MushroomFlipbookComp = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("MushroomFlipbook"));
	MushroomFlipbookComp->SetupAttachment(RootComponent);
	MushroomFlipbookComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Create the detection box
	DetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("DetectionBox"));
	DetectionBox->SetupAttachment(RootComponent);
	DetectionBox->SetBoxExtent(FVector(50.f, 50.f, 30.f));

	// Only detect overlapping Pawns
	DetectionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	DetectionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DetectionBox->SetGenerateOverlapEvents(true);

	// Bind the overlap event
	DetectionBox->OnComponentBeginOverlap.AddDynamic(
		this,
		&ACPP_JumpingMushroom::OnDetectionBoxOverlap
	);

	// Bind the animation finished event
	MushroomFlipbookComp->OnFinishedPlaying.AddDynamic(
		this,
		&ACPP_JumpingMushroom::OnBounceAnimationFinished
	);
}

// *******************************************************************************
//                                  BEGIN PLAY
// *******************************************************************************
void ACPP_JumpingMushroom::BeginPlay()
{
	Super::BeginPlay();

	// Set the mushroom's default animation
	if (JumpingMushroomIdle)
	{
		MushroomFlipbookComp->SetFlipbook(JumpingMushroomIdle);
	}

	// Make sure the default animation loops forever
	MushroomFlipbookComp->SetLooping(true);
	MushroomFlipbookComp->Play();
}

// *******************************************************************************
//                              DETECTION BOX OVERLAP
// *******************************************************************************
// ?: Detects when the player touches the mushroom and launches them upwards
void ACPP_JumpingMushroom::OnDetectionBoxOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    // Make sure the overlapping actor is the player
    ACPP_PlayerChar* Player = Cast<ACPP_PlayerChar>(OtherActor);
    if (!Player) return;

    // DEAD? DO NOTHING
    if (Player->IsDead()) return;

    // ***************************
    /*          BOUNCE          */
    // ***************************

    // Launch the player upwards
    // **NOTE: Keeps horizontal velocity but overrides vertical velocity
    Player->LaunchCharacter(
        FVector(0.f, 0.f, JumpStrength),
        false,
        true
    );

    // ***************************
    /*          SOUND           */
    // ***************************

    // Play the mushroom's bouncing sound
    if (BounceSound)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            BounceSound,
            GetActorLocation(),
            BounceSoundVolume,
            FMath::FRandRange(BounceSoundPitch.X, BounceSoundPitch.Y)
        );
    }

    // ***************************
    /*         PARTICLES        */
    // ***************************

    // Spawn Niagara particles at the mushroom's location
    if (BounceParticles)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            BounceParticles,
            GetActorLocation() + ParticleOffset,
            GetActorRotation()
        );
    }

    // ***************************
    /*         ANIMATION        */
    // ***************************

    // Play the mushroom's bouncing animation
    if (JumpingMushroomBouncing)
    {
        // Disable looping so the animation can finish
        MushroomFlipbookComp->SetLooping(false);

        // Change the current Flipbook
        MushroomFlipbookComp->SetFlipbook(JumpingMushroomBouncing);

        // Play the animation from the beginning
        MushroomFlipbookComp->PlayFromStart();
    }
}

// *******************************************************************************
//                             BOUNCE ANIMATION FINISHED
// *******************************************************************************
// ?: Restores the mushroom's default animation after bouncing
void ACPP_JumpingMushroom::OnBounceAnimationFinished()
{
	// Make sure a default Flipbook was assigned
	if (!JumpingMushroomIdle) return;

	// Restore the mushroom's original Flipbook
	MushroomFlipbookComp->SetFlipbook(JumpingMushroomIdle);

	// Enable looping again
	MushroomFlipbookComp->SetLooping(true);

	// Play the default animation from the beginning
	MushroomFlipbookComp->PlayFromStart();
}
