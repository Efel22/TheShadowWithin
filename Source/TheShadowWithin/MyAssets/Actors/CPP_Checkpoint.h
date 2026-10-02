// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPP_Checkpoint.generated.h"

class UPaperSpriteComponent;
class USoundBase;
class UPaperFlipbookComponent;
class UPaperFlipbook;
class UBoxComponent; // Required for Box Collision usage
class UNiagaraComponent; // Required for niagara particle components
class UNiagaraSystem; // Required for playing niagara systems

UCLASS()
class THESHADOWWITHIN_API ACPP_Checkpoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACPP_Checkpoint();

protected:

	// Flipbook Component
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components|Visual")
	UPaperFlipbookComponent* CurrentFlipbookComponent;

	// Inactive Flipbook
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Visual")
	UPaperFlipbook* Inactive_Flipbook;

	// Active Flipbook
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Visual")
	UPaperFlipbook* Active_Flipbook;

	// Detection Box Component
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	UBoxComponent* DetectionBox;

	// Ongoing particles
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Visual")
	UNiagaraComponent* OnGoingParticles;

	// Particles that play once they have been activated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Visual")
	UNiagaraSystem* OnActiveParticles;

	// Particles that play once they have been activated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Audio")
	USoundBase* OnActiveSound;

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
