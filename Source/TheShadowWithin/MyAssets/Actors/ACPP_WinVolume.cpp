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
	PrimaryActorTick.bCanEverTick = false;

	// ROOT
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Create the sprite component
	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
	SpriteComponent->SetupAttachment(RootComponent);

	// Niagra component setup
	WinParticlesComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("WinParticlesComponent"));
	WinParticlesComponent->SetupAttachment(RootComponent);
	WinParticlesComponent->bAutoActivate = false; 

	// Create the sphere component
	WinSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollectionSphereComponent"));
	WinSphereComponent->SetupAttachment(RootComponent);
	WinSphereComponent->SetSphereRadius(100.0f);

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