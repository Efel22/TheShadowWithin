// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include <EnhancedInputLibrary.h>
#include "CPP_PlayerController.generated.h"

/**
 * 
 */
UCLASS()
class THESHADOWWITHIN_API ACPP_PlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	//~~~Inputs

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input|Movement")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input|Movement")
	UInputAction* JumpAction;

	//~~~Funtions

	virtual void SetupInputComponent() override;

	UFUNCTION()
	void Move(const FInputActionValue& Value);

	UFUNCTION()
	void Jump();

	UFUNCTION()
	void StopJumping();
	
};
