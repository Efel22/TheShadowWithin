// Fill out your copyright notice in the Description page of Project Settings.
//Legend, Use ctrl + f to easily find code blocks utilizing key words:
/*
* Inputs
* Movement Functions
*/


#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include <EnhancedInputLibrary.h>
#include "CPP_PlayerController.generated.h"

class ACPP_PlayerChar;
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

	//~~~Player
	ACPP_PlayerChar* PlayerCharacter;

	//~~~Functions

	virtual void OnPossess(APawn* InPawn) override;

	virtual void SetupInputComponent() override;

	//~~~Movement Functions
	UFUNCTION()
	void Move(const FInputActionValue& Value);

	UFUNCTION()
	void Jump();

	UFUNCTION()
	void StopJumping();
	
};
