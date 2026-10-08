
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPP_JumpingMushroom.generated.h"

class UPaperFlipbookComponent;
class UPaperFlipbook;
class UBoxComponent;
class UNiagaraSystem;
class USoundBase;

UCLASS()
class THESHADOWWITHIN_API ACPP_JumpingMushroom : public AActor
{
	GENERATED_BODY()

public:
	ACPP_JumpingMushroom();

protected:
	virtual void BeginPlay() override;

	// ***************************************************************************************************************
	//                                              SOUND & PARTICLES
	// ***************************************************************************************************************

	// Sound played whenever the player bounces on the mushroom
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping Mushroom|Sound")
	USoundBase* BounceSound = nullptr;

	// Sound volume
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping Mushroom|Sound")
	float BounceSoundVolume = 1.0f;

	// Random pitch range
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping Mushroom|Sound")
	FVector2D BounceSoundPitch = FVector2D(0.8f, 1.2f);

	// Niagara particles spawned whenever the mushroom is activated
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping Mushroom|Particles")
	UNiagaraSystem* BounceParticles = nullptr;

	// Offset from the mushroom's position
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping Mushroom|Particles")
	FVector ParticleOffset = FVector(0.f, 0.f, 20.f);

	// ***************************************************************************************************************
	//                                                  COMPONENTS
	// ***************************************************************************************************************

	// Mushroom's visual representation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPaperFlipbookComponent* MushroomFlipbookComp;

	// Detects when the player touches the mushroom
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* DetectionBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* SceneRoot;

	// ***************************************************************************************************************
	//                                                  VARIABLES
	// ***************************************************************************************************************

	// Determines how strongly the mushroom launches the player upwards
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping Mushroom")
	float JumpStrength = 1200.f;

	// Mushroom's default animation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping Mushroom|Animations")
	UPaperFlipbook* JumpingMushroomIdle = nullptr;

	// Animation played whenever the player touches the mushroom
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Jumping Mushroom|Animations")
	UPaperFlipbook* JumpingMushroomBouncing = nullptr;

	// ***************************************************************************************************************
	//                                                  FUNCTIONS
	// ***************************************************************************************************************

	// Called whenever an actor enters the mushroom's detection box
	UFUNCTION()
	void OnDetectionBoxOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	// Called when the bouncing animation finishes playing
	UFUNCTION()
	void OnBounceAnimationFinished();
};
