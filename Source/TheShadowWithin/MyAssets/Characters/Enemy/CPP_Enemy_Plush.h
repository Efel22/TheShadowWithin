// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyAssets/Characters/Enemy/CPP_Enemy_Master.h"
#include "CPP_Enemy_Plush.generated.h"


class ACPP_PlayerChar;
class USphereComponent;
/**
 * 
 */
UCLASS()
class THESHADOWWITHIN_API ACPP_Enemy_Plush : public ACPP_Enemy_Master
{
	GENERATED_BODY()

public:

	ACPP_Enemy_Plush();

	
	// ***************************************************************************************************************
	//                                                  SOUNDS
	// ***************************************************************************************************************

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	USoundBase* Sound_Jump;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	USoundBase* Sound_HasBeenHurt;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	//USoundBase* Sound_HasBeenDefeated;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	USoundBase* Sound_MeleeAttack;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~Enemy Attack
	//Attack Collider
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* AttackCollider;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


	// *******************************************************************************
	//                                   ATTACK 
	// *******************************************************************************

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Attack")
	float attackHitBox = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Attack")
	float knockback = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Attack")
	bool bEnemyIsAttacking = false;

	//Attack Timer
	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Attack")
	float attackTime = .5f;

	void AttackPlayer();

	FTimerHandle AttackCooldownTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Attack")
	float attackCooldownTime = 1.f;

	// *******************************************************************************
	//                                    HURT 
	// *******************************************************************************
	// 
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
	void EndHurt() { bIsBeingHurt = false; }
	FTimerHandle HurtTimer;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsBeingHurt = false;

	//void AttackCooldown();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~Enemy defeat
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bHasBeenDefeated = false;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	// Player Char. reference
	UPROPERTY()
	ACPP_PlayerChar* PlayerCharRef;

	// Enemy's detection range
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components | Enemy")
	float FDetectionRange = 500.f;

	// Enemy's movement speed (FOR NOW
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components | Enemy")
	float FMovementSpeed = 250.f;

	//~~~Jumping timer
	FTimerHandle JumpTimer;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Jump")
	float jumpTime = 5.f;

	void EnemyJump();

	



private:
	// Stores the original scale in case the scale isn't 1.f
	// **NOTE: Kept for reference, but no longer used for flipping
	FVector originalScale;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	//~~~Enemy defeat
	void Defeat();

	//Setters and Getters
	//Setters
	void SetEnemyisAttacking(bool EA);

	//Getters
	bool GetEnemyIsAttacking();

};
