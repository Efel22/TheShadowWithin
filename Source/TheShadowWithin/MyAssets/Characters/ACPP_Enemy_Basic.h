// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PaperCharacter.h"
#include "ACPP_Enemy_Basic.generated.h"


//class UBoxComponent;
class ACPP_PlayerChar;
class USphereComponent;

UCLASS()
class THESHADOWWITHIN_API AACPP_Enemy_Basic : public APaperCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AACPP_Enemy_Basic();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	//Attack Collider
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	USphereComponent* AttackCollider;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Attack")
	float attackHitBox = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy|Attack")
	float knockback = 600.f;

	bool bEnemyIsAttacking = false;
	//Attack Timer
	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Attack")
	float attackTime = .5f;

	void AttackPlayer();

	FTimerHandle AttackCooldownTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy|Attack")
	float attackCooldownTime = 1.f;

	//void AttackCooldown();

	

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
	FVector originalScale;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//Setters and Getters
	void SetEnemyisAttacking(bool EA);

	bool GetEnemyIsAttacking();

};
