// Fill out your copyright notice in the Description page of Project Settings.
//Legend, Use ctrl + f to easily find code blocks utilizing key words:
/*
Movement Functions
*/

#include "MyAssets/Characters/CPP_PlayerChar.h"
#include "PaperFlipbookComponent.h" // Required for sprite flipping
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ACPP_PlayerChar::ACPP_PlayerChar() {
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->SetRelativeLocationAndRotation(FVector(0.0f, 1920.0f, 0.0f), FRotator(0.0f, -90.0f, 0.0f));
	Camera->SetProjectionMode(ECameraProjectionMode::Orthographic);
	Camera->SetAutoCalculateOrthoPlanes(false);
	Camera->SetOrthoWidth(orthoWidth);
}

void ACPP_PlayerChar::DoMove(float Forward) {
	const FVector MoveDir = FVector(1.0f, Forward > 0.0f ? 0.1f : -0.1f, 0.0f);
	AddMovementInput(MoveDir, Forward);

	// Flip the player's sprite based on the movement direction
	if (Forward != 0.0f)
	{
		FVector fixedScale = GetSprite()->GetRelativeScale3D();

		// Keep the original scale size and only change its direction
		fixedScale.X = FMath::Abs(fixedScale.X) * FMath::Sign(Forward);

		GetSprite()->SetRelativeScale3D(fixedScale);
	}
}

void ACPP_PlayerChar::DoJump() {
	Jump();
}

void ACPP_PlayerChar::DoStopJump() {
	if (GetCharacterMovement()->Velocity.Z > 0) {
		GetCharacterMovement()->Velocity.Z *= jumpCutOff;
	}
	StopJumping();
}