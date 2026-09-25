// Fill out your copyright notice in the Description page of Project Settings.
//Legend, Use ctrl + f to easily find code blocks utilizing key words:
/*
Movement Functions
*/


#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "CPP_PlayerChar.generated.h"

class UCameraComponent;

/**
 * 
 */
UCLASS()
class THESHADOWWITHIN_API ACPP_PlayerChar : public APaperCharacter
{
	GENERATED_BODY()

public:
	ACPP_PlayerChar();

protected:
	//~~~Properties
	// Player camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	//~~~Variables
	// Camera Movements
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|CameraMovements")
	float orthoWidth = 7000;

public:
	//~~~Functions

	//--------------------------------------------------------------------------------------------------------------
	//---Movement Functions
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DoMove(float Foward);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DoJump();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DoStopJump();
	//--------------------------------------------------------------------------------------------------------------
	
};
