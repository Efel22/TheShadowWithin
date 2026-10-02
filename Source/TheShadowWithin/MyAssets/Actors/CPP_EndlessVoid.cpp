// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_EndlessVoid.h"
#include "NiagaraSystem.h" // Required for niagara system usage
#include "NiagaraComponent.h" // Required for niagara component 
#include "NiagaraFunctionLibrary.h" // Required for spawning niagara systems
#include "MyAssets/Characters/CPP_PlayerChar.h" // Player ref
#include "Kismet/GameplayStatics.h" // Required for play_sound usage
#include "Components/BoxComponent.h" 
#include "PaperSpriteComponent.h"

// Sets default values
ACPP_EndlessVoid::ACPP_EndlessVoid()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// ROOT
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Setup the platform sprite comp.
	PitSpriteComp = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Pit Sprite"));
	PitSpriteComp->SetupAttachment(RootComponent);
	PitSpriteComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Disable sprite collison

	// Box Component
	DetectionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Detection Box"));
	DetectionBox->SetupAttachment(RootComponent);
	DetectionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	DetectionBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	DetectionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	DetectionBox->SetGenerateOverlapEvents(true);

	// Bind
	if (DetectionBox) DetectionBox->OnComponentBeginOverlap.AddDynamic(this, &ACPP_EndlessVoid::OnDetectionBoxBeginOverlap);

}


// *******************************************************************************
//                             DETECTION BOX OVERLAP
// *******************************************************************************
void ACPP_EndlessVoid::OnDetectionBoxBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Yellow,FString::Printf(TEXT("PIT OVERLAP: %s"),OtherActor ? *OtherActor->GetName() : TEXT("NULL")));
	}

	// Get the player character ((required to change the respawn point
	ACPP_PlayerChar* Player = Cast<ACPP_PlayerChar>(
		OtherActor
	);
	// Safety check
	if (!Player)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,5.f,FColor::Red,TEXT("Overlap happened, but actor is NOT CPP_PlayerChar!"));
		}

		return;
	}

	// DEBUG
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Blue, TEXT("CHECKPOINT: Player has entered in the detection box of the PIT!"));

	switch (PitType)
	{
	/* ITS SUPPOSED TO CALL A PLAYER.DAMAGE FUNCTION, FOR NOW, TELEPORT TO RESPAWN POINT */
	case EFloorType::Deadly:

		Player->SetActorLocation(Player->GetRespawnPoint());

		break;

	/* TELEPORTS PLAYER TO SPECIFIED POSITION */
	case EFloorType::Teleporting:

		// Teleports the player to the specified position RELATIVE (hence the GetActorLocation + ...)
		// to this actors position
		Player->SetActorLocation(TeleportPawnToPos + GetActorLocation());

		break;

	default:
		break;
	}

	// Play Sound
	if (Pit_OnEnterSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),
			Pit_OnEnterSound,
			GetActorLocation(),
			1.f,  // Volume
			FMath::FRandRange(0.8f, 1.2f) // Random Pitch
		);
	}

}