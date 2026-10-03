// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAssets/Actors/CPP_Vine.h"
#include "MyAssets/Characters/CPP_PlayerController.h"
#include "PaperSpriteComponent.h"
#include "Components/BoxComponent.h"
#include "Engine/Engine.h"
#include "Components/ArrowComponent.h" // Used to determine where the "center" of the vine is (used in player's sprite flipping)
#include "NiagaraFunctionLibrary.h" // Required for spawning niagara systems
#include "Kismet/GameplayStatics.h" // Required for play_sound usage

// Sets default values
ACPP_Vine::ACPP_Vine()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// ROOT
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Create the sprite comp.
	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Vine Sprite"));
	SpriteComponent->SetupAttachment(RootComponent);
	SpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Disable collision

	// Create the detection box
	DetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Detection Box"));
	DetectionBox->SetupAttachment(RootComponent);
	DetectionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // Disables physics collisions
	DetectionBox->SetCollisionResponseToAllChannels(ECR_Ignore); // Clear all channels
	DetectionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // Make it so it only reacts to the pawn channel
	DetectionBox->SetGenerateOverlapEvents(true); // Allows for overlapping events *IMPORANT*
	
	// Center of Vine Component
	CenterOfVineComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Center of Vine"));
	CenterOfVineComponent->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void ACPP_Vine::BeginPlay()
{
	Super::BeginPlay();

	// Bind the being and end overlaps
	DetectionBox->OnComponentBeginOverlap.AddDynamic(
		this,
		&ACPP_Vine::OnDetectionBoxBeginOverlap
	);

	DetectionBox->OnComponentEndOverlap.AddDynamic(
		this,
		&ACPP_Vine::OnDetectionBoxEndOverlap
	);
}

// Called every frame
void ACPP_Vine::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


// *******************************************************************************
//                             DETECTION BOX OVERLAP
// *******************************************************************************
void ACPP_Vine::OnDetectionBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	
	// Get the player controller
	ACPP_PlayerController* PC = Cast<ACPP_PlayerController>(
		UGameplayStatics::GetPlayerController(GetWorld(), 0)
	);
	// Safety check
	if (!PC) return;

	// DEBUG
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("VINE: Player has grabbed onto it"));
	}

	// ?: Set the HoldingVine to true on the Controller, also 
	// passes this vine's X position (used to make it so player always faces the vine)
	PC->SetIsHoldingVine(true, CenterOfVineComponent->GetComponentLocation().X);

	

	// Play Sound
	if (GrabVineSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), 
			GrabVineSound, 
			GetActorLocation(), 
			1.f,  // Volume
			FMath::FRandRange(0.8f, 1.2f) // Random Pitch
		);
	}

	// Play Particles
	if (VineInteractParticles)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),
			VineInteractParticles,
			GetActorLocation()
		);
	}
}

void ACPP_Vine::OnDetectionBoxEndOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex
)
{

	// Get the player controller
	ACPP_PlayerController* PC = Cast<ACPP_PlayerController>(
		UGameplayStatics::GetPlayerController(GetWorld(), 0)
	);
	// Safety check
	if (!PC) return;

	// ?: Set the HoldingVine to true on the Controller, no need to pass the vine's X position
	PC->SetIsHoldingVine(false);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("VINE: Player has let go of it"));
	}

	// Play sound
	if (LetGoVineSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),
			LetGoVineSound,
			GetActorLocation(),
			1.f,  // Volume
			FMath::FRandRange(0.8f, 1.2f) // Random Pitch
		);
	}

	// Play Particles
	if (VineInteractParticles)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),
			VineInteractParticles,
			GetActorLocation()
		);
	}

}