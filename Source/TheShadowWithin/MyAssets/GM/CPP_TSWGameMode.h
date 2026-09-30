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

	// Material used for the darkness post process
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Darkness")
	UMaterialInterface* DarknessMaterial = nullptr;

	// Dynamic version of the darkness material
	UPROPERTY()
	UMaterialInstanceDynamic* DarknessMaterialDynamic = nullptr;

	// Current darkness value
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Darkness", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Darkness = 0.0f;

	// How much darkess gets added each interval
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Darkness", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DarknessIncrements = 0.01f;

	// Time between each darkness increment interval
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Darkness", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float SecondsBetweenIncrements = 1.f;

	// Does the darkness increment? (Can be used for debugging)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Darkness")
	bool DoesDarknessIncrements = true;

	// Darkness Timer
	// ?: Used to increase the darkness effect overtime
	FTimerHandle DarknessTimer;

public:

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
	void StopDarknessTimer() { GetWorld()->GetTimerManager().ClearTimer(DarknessTimer); }


};