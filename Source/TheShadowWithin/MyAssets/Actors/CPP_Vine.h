// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/BoxComponent.h"
#include "CPP_Vine.generated.h"

class UArrowComponent; // Used to determine where the "center" of the vine is (used in player's sprite flipping)
class UPaperSpriteComponent; // Required for sprite handling
class USoundBase; // Required for sound base
class UNiagaraSystem; // Required for playing niagara systems

UCLASS()
class THESHADOWWITHIN_API ACPP_Vine : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACPP_Vine();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Sprite displayed in the world
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Vine|Visual")
	UPaperSpriteComponent* SpriteComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Vine");
	UBoxComponent* DetectionBox;

	// Used to determine where the "center" of the vine is (used in player's sprite flipping)
	// WHY?: A component is really easy to move around and you an use getComponentLocation(), which is, in my opinion, better
	//       than having an FVector. Why'd do this now and not in EndlessVoid.h? cuz i found out about this NOW :/ 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Vine|Other")
	UArrowComponent* CenterOfVineComponent;

	// Sound that plays when vine is grabbed
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Vine|Audio")
	USoundBase* GrabVineSound;

	// Sound that plays when vine is let go
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Vine|Audio")
	USoundBase* LetGoVineSound;

	// Particles that play when the vine is grabbed and let go of 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Vine|Visual")
	UNiagaraSystem* VineInteractParticles;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

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

	// Called when something leaves the detection box
	UFUNCTION()
	void OnDetectionBoxEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

};
