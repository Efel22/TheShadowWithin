// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPP_EndlessVoid.generated.h"

class UPaperSpriteComponent;
class USoundBase;
class UNiagaraComponent;
class UBoxComponent;

UENUM(BlueprintType)
enum class EFloorType : uint8
{
	Deadly, // Deals damage to the player
	Teleporting // Teleports the player to a specific point
};

UCLASS()
class THESHADOWWITHIN_API ACPP_EndlessVoid : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACPP_EndlessVoid();

protected:

	// Stores the pit's sprite
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Visual")
	UPaperSpriteComponent* PitSpriteComp;

	// Sound that plays when the pit is activated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Audio")
	USoundBase* Pit_OnEnterSound;

	// Stores the pit's sprite
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Visual")
	UNiagaraComponent* Pit_OnGoingParticlesComp;

	// Stores the detection box
	// ?: Once a player has been detected, teleport them to their Respawn Point
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Detection")
	UBoxComponent* DetectionBox;

	// Type of floor (pit)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Action")
	EFloorType PitType = EFloorType::Deadly;

	// Teleporting Position if EFloorType = Teleporting
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Action",
		meta = (
			MakeEditWidget = "true",
			EditCondition = "PitType == EFloorType::Teleporting",
			EditConditionHides
			)
		)
	FVector TeleportPawnToPos;

	// Called when something enters the detection box
	UFUNCTION()
	void OnDetectionBoxBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

};
