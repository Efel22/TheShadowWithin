// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyAssets/Characters/Enemy/CPP_Enemy_Master.h"
#include "CPP_EnemyActionsInterface.h"
#include "CPP_Enemy_Boss.generated.h"



class ACPP_PlayerChar;
class ACPP_Enemy_Plush;
class UCPP_BossHealthBar;
class UStaticMeshComponent;
class UBoxComponent;

/**
 * 
 */
UCLASS()
class THESHADOWWITHIN_API ACPP_Enemy_Boss : public ACPP_Enemy_Master
{
	GENERATED_BODY()

public:
	ACPP_Enemy_Boss();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	USoundBase* Sound_HasBeenHurt;

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	//USoundBase* Sound_HasBeenDefeated;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	USoundBase* Sound_MeleeAttack;

	

protected:
	virtual void BeginPlay() override;

	//~~~Static mesh that marks the location on where the enemies spawn when the boss summons them
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* EnemySpawn;

	// UI connection
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UCPP_BossHealthBar> HealthBarWidgetClass;

	UPROPERTY()
	UCPP_BossHealthBar* HealthBarWidgetInstance;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~Boss Attack
	//Attack Collider
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* AttackCollider;

	UFUNCTION()
	void OnBeginAttackOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndAttackOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	// *******************************************************************************
	//                                   ATTACK 
	// *******************************************************************************

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Attack")
	float attackHitBox = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Attack")
	float knockback = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Attack")
	bool bEnemyIsAttacking = false;

	void AttackPlayer_Implementation() override;

	void StopAttack();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Spawn")
	TArray<ACPP_Enemy_Plush*> enemy;


	//~~~Enemy defeat
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bHasBeenDefeated = false;

	void DefeatEnemy_Implementation() override;

	void CanBossSpawnEnemies_Implementation(int defeatedAlly) override;

	int remainingSpwnEnemy = 0;

	void HasSpawnLimitReached_Implementation(int summonuedAlly) override;

	int enemyLimit = 0;

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	// Player Char. reference
	UPROPERTY()
	ACPP_PlayerChar* PlayerCharRef;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Setters and Getters
	ACPP_Enemy_Plush* GetSummonedEnemy(int index);
	int GetSummonedEnemyNum();

	FVector GetSpawnLocation();

};
