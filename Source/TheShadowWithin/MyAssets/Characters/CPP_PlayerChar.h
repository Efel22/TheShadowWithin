// Fill out your copyright notice in the Description page of Project Settings.
//Legend, Use ctrl + f to easily find code blocks utilizing key words:
/*
Movement Functions
*/


#pragma once

#include "CoreMinimal.h"
#include "PaperCharacter.h"
#include "PaperZDCharacter.h"
#include "CPP_PlayerChar.generated.h"

class UCameraComponent;
class UArrowComponent; // Used to determine where the "center" of the vine is (used in player's sprite flipping)
class USoundBase; // Used to declare sound properties


/**
 *
 */
UCLASS()
class THESHADOWWITHIN_API ACPP_PlayerChar : public APaperZDCharacter
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

	//Jump
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character|Jump")
	float jumpCutOff = 0.f;


	// Used to determine where the "center" of the vine is (used in player's sprite flipping)
	// WHY?: A component is really easy to move around and you an use getComponentLocation(), which is, in my opinion, better
	//       than having an FVector. Why'd do this now and not in EndlessVoid.h? cuz i found out about this NOW :/ 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components|Other")
	UArrowComponent* CenterOfPlayerComponent;


	// ***************************************************************************************************************
	//                                                  SOUNDS
	// ***************************************************************************************************************

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	USoundBase* Sound_Jump;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	USoundBase* Sound_HasBeenHurt;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	USoundBase* Sound_HasBeenHealed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	USoundBase* Sound_HasRespawned;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	USoundBase* Sound_HasBeenDefeated;

	// ***************************************************************************************************************
	//                                                  PARTICLES
	// ***************************************************************************************************************

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Particles")
	//UNiagaraSystem;

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
	//--------------------------------------------------------------------------------------------------------------

	//~~~HEALTH FUNCTIONS & LOGIC

	// Maximu amount of lives
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	int maxAmountOfLives = 3;

	// Starts with this amount of lives
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health")
	int amountOfLives = 3;

	// How long until the player can take damage again?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health| Inmunity")
	float damageInmunityInterval = 0.25f;

	// How long until the player can take heal again?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Health| Inmunity")
	float healInmunityInterval = 0.25f;

	// HURT LOGIC 
	// Hurt         -> Calls EndHurt() using HurtTimer, 
	// EndHurt()    -> Restores damage vulnerability, 
	// bIsBeingHurt -> True when Hurt() is called, False when EndHurt() is called, 
	//	               also used by the PaperZD Anim BP to determine whether or not to play the HURT animation
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Hurt();
	void EndHurt();
	FTimerHandle HurtTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsBeingHurt = false;
	// ***********************************************************************************************************

	// HEAL LOGIC 
	// Heal           -> Calls EndHeal() using HealTimer, 
	// EndHeal()      -> Restores healing vulnerability, 
	// bIsBeingHealed -> True when Heal() is called, False when EndHeal() is called
	//	                 also used by the PaperZD Anim BP to determine whether or not to play the HEAL animation
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal();
	void EndHeal();
	FTimerHandle HealTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsBeingHealed = false;
	// ***********************************************************************************************************

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