// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAssets/Characters/Enemy/CPP_Enemy_Master.h"
#include "PaperZDCharacter.h"
#include "PaperFlipbookComponent.h"


// *******************************************************************************
//                           UPDATE SPRITE ROTATION
// *******************************************************************************
// ?: Rotates the enemy's sprite without affecting the Actor or its collision
void ACPP_Enemy_Master::UpdateSpriteRotation()
{
	// Safety check
	if (!GetSprite()) return;

	// Determine the correct Z rotation
	// ?: RIGHT = 0 degrees, LEFT = -180 degrees
	float RotationZ = FacingDirection > 0 ? 0.f : -180.f;

	// Rotate ONLY the sprite around the Z axis
	// **NOTE: FRotator uses (Pitch, Yaw, Roll)
	// **NOTE: Yaw controls rotation around the Z axis
	GetSprite()->SetRelativeRotation(
		FRotator(0.f, RotationZ, 0.f)
	);
}

void ACPP_Enemy_Master::SetFacingDirection(float FD) {
	FacingDirection = FD;
}

void ACPP_Enemy_Master::SetEnemyHP(float newEnemyHP) {
	enemyHP = newEnemyHP;
}



void ACPP_Enemy_Master::SetEnemyDamage(float newEnemyDamage) {
	enemyDamage = newEnemyDamage;
}

float ACPP_Enemy_Master::GetFacingDirection() {
	return FacingDirection;
}

float ACPP_Enemy_Master::GetEnemyHP() {
	return enemyHP;
}

float ACPP_Enemy_Master::GetEnemyDamage() {
	return enemyDamage;
}

ACPP_PatrolPoint* ACPP_Enemy_Master::GetPatrolPoint() {
	return PatrolPoint;
}

UBehaviorTree* ACPP_Enemy_Master::GetBehaviorTree() const {
	return Tree;
}