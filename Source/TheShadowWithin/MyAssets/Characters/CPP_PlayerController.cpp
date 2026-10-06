// Fill out your copyright notice in the Description page of Project Settings.
//Legend, Use ctrl + f to easily find code blocks utilizing key words:
/*
Movement Functions
*/

#include "MyAssets/Characters/CPP_PlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/Engine.h"
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
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Completed, this, &ACPP_PlayerController::StopMove_Horizontal);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACPP_PlayerController::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACPP_PlayerController::StopJumping);
		EnhancedInputComponent->BindAction(ClimbVineAction, ETriggerEvent::Triggered, this, &ACPP_PlayerController::ClimbVine);
		EnhancedInputComponent->BindAction(ClimbVineAction, ETriggerEvent::Completed, this, &ACPP_PlayerController::StopMove_Vertical);
		EnhancedInputComponent->BindAction(SwordAtkAction, ETriggerEvent::Triggered, this, &ACPP_PlayerController::SwordAttack);
		EnhancedInputComponent->BindAction(SwordDefAction, ETriggerEvent::Ongoing, this, &ACPP_PlayerController::SwordParry);
		EnhancedInputComponent->BindAction(SwordDefAction, ETriggerEvent::Triggered, this, &ACPP_PlayerController::SwordDefense);
		
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

void ACPP_PlayerController::ClimbVine(const FInputActionValue& Value) {

	
	if (!bIsHoldingVine)
		return;

	FVector2D MoveVector = Value.Get<FVector2D>();

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			0.0f,
			FColor::Yellow,
			FString::Printf(
				TEXT("CLIMB -> X: %.2f | Y: %.2f"),
				MoveVector.X,
				MoveVector.Y
			)
		);
	}
	
	// Safety Check
	if (!PlayerCharacter) return;
	PlayerCharacter->DoClimbVine(MoveVector.Y);

	/*if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, TEXT("PC -> ClimbVine"));

	if (bIsHoldingVine)
	{

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::White, TEXT("PC -> ClimbVine -> IsHoldingVine"));
		FVector2D MoveVector = Value.Get<FVector2D>();
		PlayerCharacter->DoClimbVine(MoveVector.X);
	}*/
}

// ?: Used to prevent horizontal sliding when holding onto a vine, executed
//    after letting go of the A/D keys
void ACPP_PlayerController::StopMove_Horizontal()
{
	if (bIsHoldingVine && PlayerCharacter)
	{
		PlayerCharacter->StopHorizontalMovement();
	}
}

// ?: Used to prevent vertical sliding when holding onto a vine, executed
//    after letting go of the vine
void ACPP_PlayerController::StopMove_Vertical()
{
	if (bIsHoldingVine && PlayerCharacter)
	{
		PlayerCharacter->StopVerticalMovement();
	}
}

void ACPP_PlayerController::SwordAttack() {
	PlayerCharacter->DoSwordAttack();
}

void ACPP_PlayerController::SwordParry() {
	PlayerCharacter->DoSwordParry();
}

void ACPP_PlayerController::SwordDefense() {
	PlayerCharacter->DoSwordDefense();
}

void ACPP_PlayerController::SetIsHoldingVine(bool _value, float vine_pos_x) {
	if (_value)
	{
		PlayerCharacter->StartClimbingVine();
		PlayerCharacter->SetVine_PosX(vine_pos_x);
	}
	else {
		PlayerCharacter->StopClimbingVine();
	}

	bIsHoldingVine = _value; 
}