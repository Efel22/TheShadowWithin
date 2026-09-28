// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PaperCharacter.h"
#include "ACPP_Enemy_Basic.generated.h"



class ACPP_PlayerChar;

UCLASS()
class THESHADOWWITHIN_API AACPP_Enemy_Basic : public APaperCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AACPP_Enemy_Basic();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Player Char. reference
	UPROPERTY()
	ACPP_PlayerChar* PlayerCharRef;

	// Enemy's detection range
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components | Enemy")
	float FDetectionRange = 500.f;

	// Enemy's movement speed (FOR NOW
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components | Enemy")
	float FMovementSpeed = 250.f;

private:
	// Stores the original scale in case the scale isn't 1.f
	FVector originalScale;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
