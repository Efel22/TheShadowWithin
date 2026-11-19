// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAssets/Characters/Enemy/CPP_Enemy_Boss.h"
#include "PaperZDCharacter.h"
#include "Kismet/GameplayStatics.h" // Required for getting the player character
#include "MyAssets/Characters/CPP_PlayerChar.h" // Required for player char. casting
#include "GameFramework/CharacterMovementComponent.h" // Required for changing the character's movement speed (FOR NOW)
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "PaperTileMapActor.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MyAssets/Characters/Enemy/CPP_EnemyAiController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "PaperFlipbookComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h" // Required for on-screen rotation debugging
#include "Blueprint/UserWidget.h"
#include "MyAssets/UI/CPP_BossHealthBar.h"
#include "Components/StaticMeshComponent.h"
#include "MyAssets/Characters/Enemy/CPP_Enemy_Plush.h"

ACPP_Enemy_Boss::ACPP_Enemy_Boss()
{
	AttackCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Attack Collider"));
	AttackCollider->SetupAttachment(GetSprite());
	AttackCollider->OnComponentBeginOverlap.AddDynamic(this, &ACPP_Enemy_Boss::OnBeginAttackOverlap);
	AttackCollider->OnComponentEndOverlap.AddDynamic(this, &ACPP_Enemy_Boss::OnEndAttackOverlap);

	EnemySpawn = CreateDefaultSubobject<UStaticMeshComponent>("Spawnpoint");
	EnemySpawn->SetupAttachment(GetSprite());

	PrimaryActorTick.bCanEverTick = true;
}

void ACPP_Enemy_Boss::BeginPlay()
{
	Super::BeginPlay();

	// Get the reference to the player's character
	PlayerCharRef = Cast<ACPP_PlayerChar>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)
	);

	if (HealthBarWidgetClass) {
		HealthBarWidgetInstance = CreateWidget<UCPP_BossHealthBar>(GetWorld(), HealthBarWidgetClass);
		if (HealthBarWidgetInstance) {
			HealthBarWidgetInstance->AddToViewport();
			HealthBarWidgetInstance->SetBossHealthBarMax(enemyHP);
		}
	}

	if (!enemy.IsEmpty()) {
		remainingSpwnEnemy = enemy.Num() - 1;
	}
	/*if (ACPP_EnemyAiController* enemyController = Cast<ACPP_EnemyAiController>(GetController())) {
		ACPP_Enemy_Plush* spawnedEnemy = Cast<ACPP_Enemy_Plush>(enemyController);
		if (spawnedEnemy) {
			spawnedEnemy->OnSpawnDefeat.AddDynamic(this, &ACPP_Enemy_Boss::CanBossSpawnEnemies);
			spawnedEnemy->OnSpawnCreated.AddDynamic(this, &ACPP_Enemy_Boss::HasSpawnLimitReached);
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "Event dispatcher implemented");
		}
	}*/

	

}

void ACPP_Enemy_Boss::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Safety Check
	if (!PlayerCharRef) return;


}

void ACPP_Enemy_Boss::OnBeginAttackOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	ACPP_PlayerChar* player = Cast<ACPP_PlayerChar>(OtherActor);
	if (player) {
		//GetWorldTimerManager().SetTimer(AttackTimer, this, &ACPP_Enemy_Plush::AttackPlayer, attackTime, false);
		bEnemyIsAttacking = true;
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, "You Are being attacked");
		if (ACPP_EnemyAiController* enemyController = Cast<ACPP_EnemyAiController>(GetController())) {
			enemyController->GetBlackboardComponent()->SetValueAsBool("CanAttackPlayer", true);
		}

	}
}

void ACPP_Enemy_Boss::OnEndAttackOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	ACPP_PlayerChar* player = Cast<ACPP_PlayerChar>(OtherActor);
	if (player) {
		/*if (ACPP_EnemyAiController* enemyController = Cast<ACPP_EnemyAiController>(GetController())) {
			enemyController->GetBlackboardComponent()->SetValueAsBool("CanAttackPlayer", false);
		}*/
		StopAttack();
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, "Stop");
	}
}

void ACPP_Enemy_Boss::AttackPlayer_Implementation() {
	FHitResult hit;
	FVector start = GetActorLocation();
	FVector end = GetActorLocation();
	FQuat rot = FQuat(0, 0, 0, 0);
	FCollisionShape box = FCollisionShape::MakeBox(FVector(0, 50.f, 2000.f));
	FCollisionQueryParams traceParams;
	traceParams.AddIgnoredActor(this);

	FCollisionObjectQueryParams objectType;
	//~~~Adds a specific object type to search, In this case it will be the pawn object type
	objectType.AddObjectTypesToQuery(ECC_Pawn);

	float knockbackDirection;

	//Checks the orientation of the sprite to determine the correct direction the attack should face
	// **NOTE: Uses the stored facing direction instead of reading sprite rotation
	// ?: This prevents incorrect attacks if another system modifies the sprite's rotation

	

	if (GetSprite()->GetRelativeRotation().Yaw < 0) {

		// Facing RIGHT
		end.X = GetActorLocation().X + attackHitBox;
		knockbackDirection = knockback;
	}
	else {

		// Facing LEFT
		end.X = GetActorLocation().X + (-attackHitBox);
		knockbackDirection = -(knockback);
	}

	if (GetWorld()->SweepSingleByObjectType(hit, start, end, rot, objectType, box, traceParams)) {

		ACPP_PlayerChar* player = Cast<ACPP_PlayerChar>(hit.GetActor());
		if (player) {
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, "Attacking Player");
			player->LaunchCharacter(FVector(knockbackDirection, 0, 0), true, false);
			bEnemyIsAttacking = false;
			if (!player->GetIsPlayerDefending()) {
				player->Hurt();
			}
		}
		else {
			return;
		}

		//GetWorldTimerManager().SetTimer(AttackCooldownTimer, this, &ACPP_Enemy_Plush::AttackPlayer, attackCooldownTime, false);
	}

	// Melee Attack sound
	if (Sound_MeleeAttack)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),
			Sound_MeleeAttack,
			GetActorLocation(),
			1.f,  // Volume
			FMath::FRandRange(0.8f, 1.2f) // Random Pitch
		);
	}
}



void ACPP_Enemy_Boss::StopAttack() {
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, "Attack has stopped");

	if (ACPP_EnemyAiController* enemyController = Cast<ACPP_EnemyAiController>(GetController())) {
		enemyController->GetBlackboardComponent()->SetValueAsBool("CanAttackPlayer", false);
	}

}

void ACPP_Enemy_Boss::DefeatEnemy_Implementation() {
	if (enemyHP <= 1) {
		GetSprite()->SetVisibility(false);
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		AttackCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		//StopAttack();

		if (ACPP_EnemyAiController* enemyController = Cast<ACPP_EnemyAiController>(GetController())) {
			enemyController->GetAIPerceptionComponent()->SetSenseEnabled(UAISense_Sight::StaticClass(), false);
			enemyController->GetBlackboardComponent()->SetValueAsBool("IsDead", true);

		}

		bHasBeenDefeated = true;
		HealthBarWidgetInstance->SetBossHealthBar(0);
	}
	else {
		enemyHP -= 1;
		HealthBarWidgetInstance->SetBossHealthBar(enemyHP);
	}

	
}

void ACPP_Enemy_Boss::CanBossSpawnEnemies_Implementation(int defeatedAlly) {
	if (remainingSpwnEnemy <= 1) {
		if (ACPP_EnemyAiController* enemyController = Cast<ACPP_EnemyAiController>(GetController())) {
			enemyController->GetBlackboardComponent()->SetValueAsBool("CanSpawnBackUp", false);
		}
		enemyLimit = 0;
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, "Enemies are dead meat");
	}
	else {
		remainingSpwnEnemy -= defeatedAlly;
		GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Green, FString::FromInt(remainingSpwnEnemy));
	}
}

void ACPP_Enemy_Boss::HasSpawnLimitReached_Implementation(int summonuedAlly) {
	if (enemyLimit >= enemy.Num() - 1) {
		if (ACPP_EnemyAiController* enemyController = Cast<ACPP_EnemyAiController>(GetController())) {
			enemyController->GetBlackboardComponent()->SetValueAsBool("CanSpawnBackUp", true);
		}
		
		remainingSpwnEnemy = enemy.Num();
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, "Spawn Limit has been reached");
	}
	else {
		enemyLimit += summonuedAlly;
		GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, FString::FromInt(enemyLimit));
	}
}

ACPP_Enemy_Plush* ACPP_Enemy_Boss::GetSummonedEnemy(int index) {

	// Check if the index is valid
	if (!enemy.IsValidIndex(index))
	{
		
		return NULL;
	}

	return enemy[index];
}

int ACPP_Enemy_Boss::GetSummonedEnemyNum() {
	return enemy.Num();
}

FVector ACPP_Enemy_Boss::GetSpawnLocation() {
	return EnemySpawn->GetComponentLocation();
}