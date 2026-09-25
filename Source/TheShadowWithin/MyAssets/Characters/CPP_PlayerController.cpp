// Fill out your copyright notice in the Description page of Project Settings.
//Legend, Use ctrl + f to easily find code blocks utilizing key words:
/*
Movement Functions
*/

#include "MyAssets/Characters/CPP_PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "MyAssets/Characters/CPP_PlayerChar.h"

void ACPP_PlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// Setting the character that we are controlling
	if (InPawn)
	{
		PlayerCharacter = Cast<ACPP_PlayerChar>(InPawn);
	}


}

void ACPP_PlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	// Adding the mapping context
	if (UEnhancedInputLocalPlayerSubsystem* InputSystem = ULocalPlayer::GetSubsystem<
		UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (InputMappingContext)
		{
			InputSystem->AddMappingContext(InputMappingContext, 0);
		}
	}

	// Setting up the basic input actions
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACPP_PlayerController::Move);
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Started, this, &ACPP_PlayerController::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACPP_PlayerController::StopJumping);
		
	}
}

//~~~Movement Functions
void ACPP_PlayerController::Move(const FInputActionValue& Value) {
	FVector2D MoveVector = Value.Get<FVector2D>();
	PlayerCharacter->DoMove(MoveVector.Y);
}

void ACPP_PlayerController::Jump() {
	PlayerCharacter->DoJump();
}

void ACPP_PlayerController::StopJumping() {
	PlayerCharacter->DoStopJump();
}