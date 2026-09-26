// Fill out your copyright notice in the Description page of Project Settings.
//Legend, Use ctrl + f to easily find code blocks utilizing key words:
/*
Movement Functions
*/

#include "MyAssets/Characters/CPP_PlayerChar.h"
#include "Camera/CameraComponent.h"

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
}

void ACPP_PlayerChar::DoJump() {
	Jump();
}

void ACPP_PlayerChar::DoStopJump() {
	StopJumping();
}