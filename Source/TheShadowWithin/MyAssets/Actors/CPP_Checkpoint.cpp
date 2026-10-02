// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAssets/Actors/CPP_Checkpoint.h"
#include "PaperFlipbookComponent.h"
#include "NiagaraSystem.h" // Required for niagara system usage
#include "NiagaraComponent.h" // Required for niagara component 
#include "NiagaraFunctionLibrary.h" // Required for spawning niagara systems
#include "MyAssets/Characters/CPP_PlayerChar.h" // Player ref
#include "Kismet/GameplayStatics.h" // Required for play_sound usage
#include "Components/BoxComponent.h" 

// Sets default values
ACPP_Checkpoint::ACPP_Checkpoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// ROOT
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Flipbook component
	CurrentFlipbookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Current Flipbook"));
	CurrentFlipbookComponent->SetupAttachment(RootComponent);
	CurrentFlipbookComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// On Going Particles Component
	OnGoingParticles = CreateDefaultSubobject<UNiagaraComponent>(TEXT("On Going Particles"));
	OnGoingParticles->SetupAttachment(RootComponent);

	// Box Component
	DetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Detection Box"));
	DetectionBox->SetupAttachment(RootComponent);
	DetectionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	DetectionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	// BIND
	if (DetectionBox)
	{
		DetectionBox->OnComponentBeginOverlap.AddDynamic(this, &ACPP_Checkpoint::OnDetectionBoxBeginOverlap);
	}

	// Set the flipbook to the inactive one
	if (Inactive_Flipbook) CurrentFlipbookComponent->SetFlipbook(Inactive_Flipbook);

}

// *******************************************************************************
//                             DETECTION BOX OVERLAP
// *******************************************************************************
void ACPP_Checkpoint::OnDetectionBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{

	// Get the player character ((required to change the respawn point
	ACPP_PlayerChar* Player = Cast<ACPP_PlayerChar>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)
	);
	// Safety check
	if (!Player) return;

	// DEBUG
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("CHECKPOINT: Player haas entered in the detection box of the Checkpoint!"));
	}

	// * * * CHECK IF THE PLAYER'S RESPAWN LOCATION ISNT THIS ACTOR'S LOCATION * * *
	if (Player->GetRespawnPoint() == GetActorLocation())
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("CHECKPOINT: Player's respawn location is the same as THIS actor's!"));
		return;
	}

	// ?: Set the respawn location to this actor's location
	Player->SetRespawnPoint(GetActorLocation());

	// Play Sound
	if (OnActiveSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),
			OnActiveSound,
			GetActorLocation(),
			1.f,  // Volume
			FMath::FRandRange(0.8f, 1.2f) // Random Pitch
		);
	}

	// Play Particles
	if (OnActiveParticles)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),
			OnActiveParticles,
			GetActorLocation()
		);
	}


	if (Active_Flipbook) CurrentFlipbookComponent->SetFlipbook(Active_Flipbook);
}
