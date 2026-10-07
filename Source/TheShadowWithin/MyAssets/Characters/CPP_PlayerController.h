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

	UPROPERTY(EditAnywhere, Category = "Input|Movement")
	UInputAction* ClimbVineAction;

	UPROPERTY(EditAnywhere, Category = "Input|Movement")
	UInputAction* SwordAtkAction;

	UPROPERTY(EditAnywhere, Category = "Input|Movement")
	UInputAction* SwordDefAction;

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

	UFUNCTION()
	void SwordAttack();

	UFUNCTION()
	void SwordParry();

	UFUNCTION()
	void SwordDefense();

	UFUNCTION()
	void StopSwordDefense();

	// ~~~ Vine Logic
	UFUNCTION()
	void ClimbVine(const FInputActionValue& Value);

	// Stops movement in the horizontal axis
	// ?: Used to prevent sliding when A/D keys are released when hanging on a vine
	UFUNCTION()
	void StopMove_Horizontal();

	// Stops movement in the vertical axis
	// ?: Used to prevent sliding when W/S keys are released when hanging on a vine
	UFUNCTION()
	void StopMove_Vertical();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsHoldingVine = false;

public:

	UFUNCTION(BlueprintCallable)
	void SetIsHoldingVine(bool _value, float vine_pos_x = 0.0f);

	UFUNCTION(BlueprintCallable)
	bool GetIsHoldingVine() { return bIsHoldingVine; }
};
