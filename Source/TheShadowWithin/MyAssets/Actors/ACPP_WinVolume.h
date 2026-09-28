// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ACPP_WinVolume.generated.h"

class UPaperSpriteComponent; // Required for sprite usage
class UBoxComponent; // Required for Box Collision usage
class UUserWidget; // Required for widgets
class USphereComponent;

UCLASS()
class THESHADOWWITHIN_API AACPP_WinVolume : public AActor
{
	GENERATED_BODY()

public:

	// Sets default values for this actor's properties
	AACPP_WinVolume();

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Sprite displayed in the world
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Win Volume")
	UPaperSpriteComponent* SpriteComponent;

	// Area used to detect when the player gets close enough
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Win Volume")
	USphereComponent* WinSphereComponent;

	//// Box Collision used to determine whether player has won or not
	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Win Volume")
	//UBoxComponent* WinBox;

	// Widget used to display after entering this box
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components|Win Volume")
	TSubclassOf<UUserWidget> WinWidgetClass;

	// Called whenever something enters the WinBox
	UFUNCTION()
	void OnWinSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

private:

	// Prevents the win screen from appearing multiple times (spam)
	bool bHasWon = false;
};