// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyAssets/Actors/ACPP_Consumable.h" // REQUIRES complete definition for CPP
#include "CPP_Fairy.generated.h"

class UPaperFlipbookComponent;
class USoundBase;
class UNiagaraSystem;
class UNiagaraComponent;
class ACPP_PlayerChar;

UENUM(BlueprintType)
enum class EFairyState : uint8
{
	Following, // Follows and flies around the player
	Guiding,   // Flies towards the nearest consumable
	Returning  // Returns to the player
};

UCLASS()
class THESHADOWWITHIN_API ACPP_Fairy : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACPP_Fairy();

protected:

	// Stores the fairy's flipbook
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Visual")
	UPaperFlipbookComponent* FairyFlipbookComp;

	// Stores the fairy's ongoing particles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Visual")
	UNiagaraComponent* Fairy_OnGoingParticlesComp;

	// Stores the particles used to guide the player towards the nearest consumable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Guidance")
	UNiagaraSystem* Fairy_GuidanceParticles;

	// Sound that plays when the fairy points towards a consumable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Audio")
	USoundBase* Fairy_GuidanceSound;


	// *******************************************************************************
	//                                  MOVEMENT
	// *******************************************************************************

	// Stores the player the fairy will follow
	UPROPERTY()
	ACPP_PlayerChar* Player;

	// How fast the fairy follows the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Movement")
	float FollowSpeed = 3.f;

	// How far the fairy flies around the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Movement")
	float OrbitRadius = 100.f;

	// How fast the fairy flies around the player
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Movement")
	float OrbitSpeed = 2.f;

	// Stores the Y position of the fairy
	// ?: The fairy should NEVER move in the Y axis
	float LockedY;

	// Stores what the fairy is currently doing
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Movement")
	EFairyState FairyState = EFairyState::Following;

	// How fast the fairy flies towards consumables
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Movement")
	float GuidanceFlySpeed = 5.f;

	// How close the fairy needs to get to a consumable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Guidance")
	float ConsumableReachDistance = 50.f;

	// How long the fairy waits at the consumable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Guidance")
	float GuidanceWaitTime = 1.5f;

	// How close the fairy needs to be to the player before returning to normal
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Movement")
	float ReturnDistance = 150.f;

	// Timer used before returning to the player
	FTimerHandle ReturnTimer;

	// *******************************************************************************
	//                                  GUIDANCE
	// *******************************************************************************

	// Stores the nearest consumable to the player
	UPROPERTY()
	ACPP_Consumable* NearestConsumable;

	// Maximum distance at which the fairy can detect consumables
// ?: Consumables outside this radius will be ignored
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Guidance", meta = (ClampMin = "0.0"))
	float SearchRadius = 500.f;

	// How often the fairy searches for the nearest consumable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Guidance")
	float SearchInterval = 0.5f;

	// How often the fairy points towards the nearest consumable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Guidance")
	float GuidanceInterval = 3.f;

	// Timer used to search for the nearest consumable
	FTimerHandle SearchTimer;

	// Timer used to activate the fairy's guidance
	FTimerHandle GuidanceTimer;


	// *******************************************************************************
	//                                  FUNCTIONS
	// *******************************************************************************

	// Makes the fairy follow and fly around the player
	void FollowPlayer(float DeltaTime);

	// Flips the fairy's sprite based on the direction its flying
	void FlipFairy(float DirectionX);

	// Finds the closest consumable to the player
	void FindNearestConsumable();

	// Spawns particles pointing towards the nearest consumable
	void GuidePlayerToConsumable();

	// Makes the fairy fly towards the nearest consumable
	void FlyToConsumable(float DeltaTime);

	// Makes the fairy return to the player
	void ReturnToPlayer();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};