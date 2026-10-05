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
class ACPP_Enemy_Basic;

class UArrowComponent; // Used to determine where the "center" of the vine is (used in player's sprite flipping)
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
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//~~~Properties
	// Player camera
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* Camera;

	//~~~Variables
	// Camera Movements
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|CameraMovements")
	float orthoWidth = 7000;

	//	Sword atributes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Sword Attack")
	float swordHitBoxSize = 300;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Sword Defense")
	float swordDefenseBoxSize = 150;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Sword Defense")
	float parryKnockback = 1000;

	//~~~Jump
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Jump")
	float jumpCutOff = 0.f;


	// Used to determine where the "center" of the vine is (used in player's sprite flipping)
	// WHY?: A component is really easy to move around and you an use getComponentLocation(), which is, in my opinion, better
	//       than having an FVector. Why'd do this now and not in EndlessVoid.h? cuz i found out about this NOW :/ 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Other")
	UArrowComponent* CenterOfPlayerComponent;

public:
	//~~~Functions

	//--------------------------------------------------------------------------------------------------------------
	//---Movement Functions
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DoMove(float Forward);

	UFUNCTION(BlueprintCallable, Category = "Movement|Vine")
	void DoClimbVine(float Forward);

	UFUNCTION(BlueprintCallable, Category = "Movement|Vine")
	void StartClimbingVine();

	UFUNCTION(BlueprintCallable, Category = "Movement|Vine")
	void StopClimbingVine();

	UFUNCTION(BlueprintCallable, Category = "Movement|Vine")
	void StopHorizontalMovement();

	UFUNCTION(BlueprintCallable, Category = "Movement|Vine")
	void StopVerticalMovement();

	UFUNCTION(BlueprintCallable, Category = "Movement|Vine")
	void FaceVine();

	UFUNCTION()
	void SetVine_PosX(float _value) { currentVine_PosX = _value; }

	// Stores the last vine's x position that was grabed
	float currentVine_PosX = 0.f;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DoJump();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DoStopJump();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DoSwordAttack();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DoSwordParry();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DoSwordDefense();
	//--------------------------------------------------------------------------------------------------------------

	//~~~HEALTH FUNCTIONS

	UFUNCTION(BlueprintCallable, Category = "Health")
	void Die();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	bool isDead = false;

	UFUNCTION(BlueprintCallable, Category = "Health")
	bool IsDead() { return isDead; }

	//--------------------------------------------------------------------------------------------------------------

	//~~~RESPAWN LOGIC

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Respawn|Location")
	FVector RespawnLocation = FVector::ZeroVector;

	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void SetRespawnPoint(FVector _value) { RespawnLocation = _value; }

	UFUNCTION(BlueprintCallable, Category = "Respawn")
	FVector GetRespawnPoint() { return RespawnLocation; }


};