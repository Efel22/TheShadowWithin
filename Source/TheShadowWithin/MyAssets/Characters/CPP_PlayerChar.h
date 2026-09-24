// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "CPP_PlayerChar.generated.h"

/**
 * 
 */
UCLASS()
class THESHADOWWITHIN_API ACPP_PlayerChar : public APaperCharacter
{
	GENERATED_BODY()

public:
	//~~~Functions

	//---Movement Functions
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DoMove(float Foward);

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DoJump();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DoStopJump();
	
};
