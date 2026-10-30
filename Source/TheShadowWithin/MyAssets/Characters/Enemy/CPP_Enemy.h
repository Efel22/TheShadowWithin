// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BehaviorTree/BehaviorTree.h"
#include "CPP_Enemy.generated.h"

class ACPP_PlayerChar;

UCLASS()
class THESHADOWWITHIN_API ACPP_Enemy : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACPP_Enemy();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ai", meta = (AllowPrivateAccess = "true"))
	UBehaviorTree* Tree;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HP")
	float enemyHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage")
	float enemyDamage;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	//~~~Setters and Getters
	
	void SetEnemyHP(float newEnemyHP);

	float GetEnemyHP();
	void SetEnemyDamage(float newEnemyDmage);

	float GetEnemyDamage();

};
