#pragma once

#include "CoreMinimal.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h" // Required for play_sound usage
#include "SoundData.generated.h"

USTRUCT(BlueprintType)
struct FSoundData
{
	GENERATED_BODY()

	// Sound asset
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* Sound = nullptr;

	// //Sound volume
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	//float Volume = 1.0f;

	//// Minimum random pitch
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	//float PitchMin = 0.8f;

	//// Maximum random pitch
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	//float PitchMax = 1.2f;

	// Returns a random pitch between PitchMin and PitchMax
	/*float GetPitch() const
	{
		return FMath::FRandRange(PitchMin, PitchMax);
	}*/

	// Play the sound
	void Play(UObject* WorldContextObject, FVector Location) const
	{
		if (!Sound || !WorldContextObject) return;

		UGameplayStatics::PlaySoundAtLocation(
			WorldContextObject,
			Sound,
			Location
		);
	}
};