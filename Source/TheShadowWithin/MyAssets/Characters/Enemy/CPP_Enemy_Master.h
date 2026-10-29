// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "MyAssets/AI/Enemy/CPP_PatrolPoint.h"
#include "CPP_EnemyActionsInterface.h"
#include "CPP_Enemy_Master.generated.h"

class ACPP_PlayerChar;


/**
 * 
 */
UCLASS()
class THESHADOWWITHIN_API ACPP_Enemy_Master : public APaperZDCharacter, public ICPP_EnemyActionsInterface
{
	GENERATED_BODY()
	
public:
	//ACPP_Enemy_Master();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ai", meta = (AllowPrivateAccess = "true"))
	UBehaviorTree* Tree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ai", meta = (AllowPrivateAccess = "true"))
	ACPP_PatrolPoint* PatrolPoint;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HP")
	float enemyHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float enemyDamage;

	//void AttackPlayer_Implementation() override;

	// *******************************************************************************
	//                             SPRITE ROTATION
	// *******************************************************************************

	// Stores the enemy's current facing direction
	// ?: 1 = RIGHT, -1 = LEFT
	// **NOTE: This value is independent of the sprite's rotation
	int32 FacingDirection = 1;

	// Enables on-screen debug information for the enemy's rotation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components | Debug")
	bool bDebugSpriteRotation = false;

public:
	
	// *******************************************************************************
	//                             SPRITE ROTATION
	// *******************************************************************************

	// Rotates the enemy's sprite based on its facing direction
	// ?: Prevents the Actor and its collision capsule from rotating
	void UpdateSpriteRotation();

	//~~~Setters and Getters

	void SetEnemyHP(float newEnemyHP);

	void SetFacingDirection(float FD);

	
	void SetEnemyDamage(float newEnemyDmage);

	float GetEnemyHP();
	float GetFacingDirection();
	float GetEnemyDamage();

	ACPP_PatrolPoint* GetPatrolPoint();

	UBehaviorTree* GetBehaviorTree() const;

};
