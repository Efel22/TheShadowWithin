
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CPP_TSWGameMode.generated.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS()
class THESHADOWWITHIN_API ACPP_TSWGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	ACPP_TSWGameMode();

protected:

	// Called when the game starts
	virtual void BeginPlay() override;

	// Called every frame while a tint transition is active
	virtual void Tick(float DeltaTime) override;

	// *******************************************************************************
	//                             POST PROCESS MATERIAL
	// *******************************************************************************

	// Material used for the darkness and tint post process
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Darkness")
	UMaterialInterface* DarknessMaterial = nullptr;

	// Dynamic version of the post process material
	UPROPERTY()
	UMaterialInstanceDynamic* DarknessMaterialDynamic = nullptr;

	// *******************************************************************************
	//                             DARKNESS
	// *******************************************************************************

	// Current darkness value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Darkness",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Darkness = 0.0f;

	// How much darkness gets added each interval
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Darkness",
		meta = (
			ClampMin = "0.0",
			ClampMax = "1.0",
			EditCondition = "DoesDarknessIncrements == true"
			))
	float DarknessIncrements = 0.01f;

	// Time between each darkness increment interval
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Darkness",
		meta = (EditCondition = "DoesDarknessIncrements == true"))
	float SecondsBetweenIncrements = 1.0f;

	// Does the darkness increment? (Can be used for debugging)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Darkness")
	bool DoesDarknessIncrements = true;

	// Darkness Timer
	// ?: Used to increase the darkness effect overtime
	FTimerHandle DarknessTimer;

	// Prevents repeatedly triggering death at maximum darkness
	bool bDarknessDeathTriggered = false;

	// *******************************************************************************
	//                             SCREEN TINT
	// *******************************************************************************

	// Current tint color applied to the screen
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process | Tint")
	FLinearColor TintColor = FLinearColor::White;

	// Current tint intensity
	// ?: 0 = no tint, 1 = full tint effect
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Post Process | Tint",
		meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TintStrength = 0.0f;

	// Starting values for the current transition
	FLinearColor InitialTintColor = FLinearColor::White;
	float InitialTintStrength = 0.0f;

	// Target values for the current transition
	FLinearColor TargetTintColor = FLinearColor::White;
	float TargetTintStrength = 0.0f;

	// Duration and elapsed time of the transition
	float TintTransitionDuration = 0.0f;
	float TintTransitionElapsed = 0.0f;

	// Determines whether a tint transition is active
	bool bIsTintTransitioning = false;

	// Updates the tint transition using linear interpolation
	void UpdateTintTransition(float DeltaTime);

	// Updates the tint parameters in the material
	void UpdateTintMaterial();

public:

	// *******************************************************************************
	//                             DARKNESS FUNCTIONS
	// *******************************************************************************

	// Increases the darkness
	UFUNCTION(BlueprintCallable, Category = "Darkness")
	void AddDarkness(float Amount);

	// Decreases the darkness
	UFUNCTION(BlueprintCallable, Category = "Darkness")
	void RemoveDarkness(float Amount);

	// Starts the darkness timer
	UFUNCTION(BlueprintCallable, Category = "Darkness")
	void StartDarknessTimer();

	// Stops the darkness timer
	UFUNCTION(BlueprintCallable, Category = "Darkness")
	void StopDarknessTimer()
	{
		GetWorldTimerManager().ClearTimer(DarknessTimer);
	}

	// *******************************************************************************
	//                             SCREEN TINT FUNCTIONS
	// *******************************************************************************

	// Changes the screen tint over the specified duration
	// ?: A duration of 0 changes the tint instantly
	UFUNCTION(BlueprintCallable, Category = "Post Process | Tint")
	void SetTint(
		FLinearColor NewColor,
		float NewStrength,
		float TransitionDuration
	);

	// Restores the original screen colors
	UFUNCTION(BlueprintCallable, Category = "Post Process | Tint")
	void ResetTint(float TransitionDuration = 1.0f);
};
