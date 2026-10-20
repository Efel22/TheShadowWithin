// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAssets/Actors/ACPP_WinVolume.h"
#include "PaperSpriteComponent.h" // Required for sprite usage
#include "MyAssets/Characters/CPP_PlayerChar.h" // Required for player char. detection
//#include "Components/BoxComponent.h" // Required for box col. component usage
#include "MyAssets/Characters/CPP_PlayerController.h" // Required for player controller functionality
#include "Blueprint/UserWidget.h" // Required to show specified widget from header file
#include "Components/SphereComponent.h" // Required for sphere col. component usage
#include "Kismet/GameplayStatics.h" // Required for play_sound usage
#include "NiagaraSystem.h" // Required for niagara system usage
#include "NiagaraComponent.h" // Required for niagara component 
#include "NiagaraFunctionLibrary.h" // Required for spawning niagara systems

// *******************************************************************************
//                             CONSTRUCTOR
// *******************************************************************************
AACPP_WinVolume::AACPP_WinVolume()
{
	// Enable ticking, for now
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// ROOT
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Create the sprite component
	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
	SpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SpriteComponent->SetupAttachment(RootComponent);

	// Niagra component setup
	WinParticlesComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WinParticlesComponent"));
	WinParticlesComponent->SetupAttachment(RootComponent);

	// Create the sphere component
	WinSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollectionSphereComponent"));
	WinSphereComponent->SetupAttachment(RootComponent);

	// Enable overlap detection
	WinSphereComponent->SetGenerateOverlapEvents(true);

	// Call our function when something enters the sphere
	WinSphereComponent->OnComponentBeginOverlap.AddDynamic(
		this,
		&AACPP_WinVolume::OnWinSphereOverlap
	);
}

// *******************************************************************************
//                             BEGIN PLAY
// *******************************************************************************
void AACPP_WinVolume::BeginPlay()
{
	Super::BeginPlay();


	// Store the sprite's original position
	if (SpriteComponent)
	{
		OriginalSpriteLocation = SpriteComponent->GetRelativeLocation();
	}

	// Only enable Tick if bobbing is enabled
	SetActorTickEnabled(bShouldBob);

	// DEBUG
	UE_LOG(LogTemp, Warning, TEXT("WIN VOLUME: %s | Bobbing: %s | Tick: %s"),
		*GetName(),
		bShouldBob ? TEXT("TRUE") : TEXT("FALSE"),
		IsActorTickEnabled() ? TEXT("TRUE") : TEXT("FALSE")
	);
}

// *******************************************************************************
//                             TICK
// *******************************************************************************
void AACPP_WinVolume::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update the sprite's floating animation
	if (bShouldBob)
	{
		UpdateBobbing(DeltaTime);

		// DEBUG: Show the current sprite position
		if (GEngine && SpriteComponent)
		{
			GEngine->AddOnScreenDebugMessage(
				(uint64)GetUniqueID(),
				0.0f,
				FColor::Green,
				FString::Printf(
					TEXT("WIN VOLUME BOBBING | Z: %.2f"),
					SpriteComponent->GetRelativeLocation().Z
				)
			);
		}
	}
}


// *******************************************************************************
//                             FLOATING ANIMATION
// *******************************************************************************
void AACPP_WinVolume::UpdateBobbing(float DeltaTime)
{
	if (!SpriteComponent)
	{
		return;
	}

	// Increase elapsed time
	BobTime += DeltaTime;

	// Calculate smooth vertical movement using a sine wave
	// Sine automatically eases at the highest and lowest points
	const float BobOffset = FMath::Sin(
		BobTime * BobFrequency * 2.0f * PI
	) * BobHeight;

	// Calculate the new position relative to the sprite's original position
	FVector NewLocation = OriginalSpriteLocation;

	// Only modify the Z axis (vertical movement)
	NewLocation.Z += BobOffset;

	// Move ONLY the sprite component
	SpriteComponent->SetRelativeLocation(NewLocation);
}

// *******************************************************************************
//                             SPHERE DETECTION
// *******************************************************************************
void AACPP_WinVolume::OnWinSphereOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	// Don't trigger more than once
	if (bHasWon)
	{
		return;
	}

	// Check if the actor entering the box is the player
	ACPP_PlayerChar* Player = Cast<ACPP_PlayerChar>(OtherActor);

	if (!Player)
	{
		return;
	}

	// Make sure a Win Widget was assigned in the Editor
	if (!WinWidgetClass)
	{
		return;
	}

	// Create the assigned Win Widget
	UUserWidget* WinWidget = CreateWidget<UUserWidget>(
		GetWorld(),
		WinWidgetClass
	);

	// Make sure the Widget was successfully created
	if (!WinWidget)
	{
		return;
	}

	// Add the Win Widget to the player's screen
	WinWidget->AddToViewport();

	// Get the player's controller
	ACPP_PlayerController* PlayerController = Cast<ACPP_PlayerController>(
		Player->GetController()
	);

	if (PlayerController)
	{
		// Make the mouse cursor visible
		PlayerController->bShowMouseCursor = true;

		// Set the player's input to UI only
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(WinWidget->TakeWidget());

		PlayerController->SetInputMode(InputMode);
	}

	// Play the sound
	if (WinSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),
			WinSound,
			GetActorLocation(),
			1.0f, // VOLUME
			FMath::FRandRange(0.8f, 1.2f) // PITCH
		);
	}

	// Play the particles
	if (WinParticles) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), WinParticles, GetActorLocation());

	// Prevent the WinBox from triggering again
	bHasWon = true;

}