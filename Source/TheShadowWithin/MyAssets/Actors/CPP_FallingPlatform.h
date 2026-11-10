
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPP_FallingPlatform.generated.h"

class USceneComponent;
class UBoxComponent;
class UPaperTileMapComponent;
class UNiagaraComponent;
class USoundBase;

UCLASS()
class THESHADOWWITHIN_API ACPP_FallingPlatform : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ACPP_FallingPlatform();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// *******************************************************************************
	//                                  COMPONENTS
	// *******************************************************************************

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* DefaultSceneRoot;

	// Detects when the player crosses the bridge
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* PlatformDeactivationTriggerBox;

	// Handles the bridge's collision and physical movement
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UBoxComponent* PlatformHitbox;

	// Visual representation of the bridge
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPaperTileMapComponent* PlatformTileMap;

	// Particles emitted while the bridge falls
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* FallingParticles;

	// Particles emitted once when the bridge lands
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UNiagaraComponent* LandingParticles;

	// *******************************************************************************
	//                               FALLING PROPERTIES
	// *******************************************************************************

	// Delay before the bridge begins falling
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform|Falling", meta = (ClampMin = "0.0"))
	float FallDelay = 0.5f;

	// Prevents activating the bridge multiple times
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform|Falling")
	bool bHasBeenTriggered = false;

	// Determines whether the bridge is falling
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform|Falling")
	bool bIsFalling = false;

	// Determines whether the bridge has landed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Platform|Falling")
	bool bHasLanded = false;

	FTimerHandle FallTimer;

	// *******************************************************************************
	//                                     SOUNDS
	// *******************************************************************************

	// Sound played when the player crosses the trigger
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform|Sounds")
	USoundBase* Sound_Triggered = nullptr;

	// Sound played when the bridge begins falling
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform|Sounds")
	USoundBase* Sound_Falling = nullptr;

	// Sound played when the bridge hits the ground
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Platform|Sounds")
	USoundBase* Sound_Landed = nullptr;

	// *******************************************************************************
	//                                   FUNCTIONS
	// *******************************************************************************

	// Starts the falling process
	void StartFalling();

	// Handles the bridge landing
	void LandPlatform(const FVector& ImpactLocation);

	// Called when the player enters the trigger
	UFUNCTION()
	void OnTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	// Called when the platform collides with another object
	UFUNCTION()
	void OnPlatformHit(
		UPrimitiveComponent* HitComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		FVector NormalImpulse,
		const FHitResult& Hit
	);

	// Called when the landing particles finish playing
	UFUNCTION()
	void OnLandingParticlesFinished(UNiagaraComponent* FinishedComponent);
};
