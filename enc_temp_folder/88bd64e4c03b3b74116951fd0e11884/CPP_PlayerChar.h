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

	// Returns the player's camera 
	// **NOTE: (USED IN THE GAMEMODE FOR THE DARKNESS EFFECT)
	UCameraComponent* GetCamera() const { return Camera; }

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
	void DoMove(float Forward);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DoClimbVine(float Forward);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DoJump();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DoStopJump();
	//--------------------------------------------------------------------------------------------------------------
	
	//~~~HEALTH FUNCTIONS

	UFUNCTION(BlueprintCallable, Category = "Health")
	void Die();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	bool isDead = false;

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool IsDead() { return isDead; }

};
