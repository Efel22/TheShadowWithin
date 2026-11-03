// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAssets/Characters/Enemy/CPP_Enemy_Plush.h"
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

// *******************************************************************************
//                             CONSTRUCTOR
// *******************************************************************************
// ?: Looks pretty :D
ACPP_Enemy_Plush::ACPP_Enemy_Plush()
{
	AttackCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Attack Collider"));
	AttackCollider->SetupAttachment(GetSprite());
	AttackCollider->OnComponentBeginOverlap.AddDynamic(this, &ACPP_Enemy_Plush::OnBeginAttackOverlap);
	AttackCollider->OnComponentEndOverlap.AddDynamic(this, &ACPP_Enemy_Plush::OnEndAttackOverlap);

	JumpCollider = CreateDefaultSubobject<UBoxComponent>(TEXT("Jump Collider"));
	JumpCollider->SetupAttachment(GetSprite());
	JumpCollider->OnComponentBeginOverlap.AddDynamic(this, &ACPP_Enemy_Plush::OnBeginJumpOverlap);
	JumpCollider->OnComponentEndOverlap.AddDynamic(this, &ACPP_Enemy_Plush::OnEndJumpOverlap);

	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// ***************************
	/*    ROTATION SETTINGS     */
	// ***************************

	// Prevent the controller from rotating the enemy automatically
	// ?: The enemy handles its own sprite rotation
	bUseControllerRotationYaw = false;

	// Prevent CharacterMovement from rotating the entire Actor
	// **NOTE: Only the sprite should rotate, NOT the collision capsule
	GetCharacterMovement()->bOrientRotationToMovement = false;

	// Prevent CharacterMovement from using controller rotation
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
}

// *******************************************************************************
//                             BEGIN PLAY
// *******************************************************************************
// ?: Assigns references, movement speed, and originalScale
void ACPP_Enemy_Plush::BeginPlay()
{
	Super::BeginPlay();

	// Get the reference to the player's character
	PlayerCharRef = Cast<ACPP_PlayerChar>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)
	);

	// Set the movement speed
	// (*FOR NOW* ILL BE USING THIS SO ALL PROPERTIES ARE FOUND IN THE SAME TAB INSIDE THE EDITOR,)
	GetCharacterMovement()->MaxWalkSpeed = FMovementSpeed;

	// Store the original scale
	// **NOTE: Kept for reference, but sprite flipping now uses rotation
	originalScale = GetActorScale();

	// ***************************
	/*    ROTATION SETTINGS     */
	// ***************************

	// Prevent CharacterMovement from rotating the enemy automatically
	// ?: Also applied here to override Blueprint defaults at runtime
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	bUseControllerRotationYaw = false;

	// Set the sprite's initial facing direction
	UpdateSpriteRotation();

	startingLocation = GetActorLocation();

	// Start the enemy's jumping timer
	/*
	GetWorldTimerManager().SetTimer(
		JumpTimer,
		this,
		&ACPP_Enemy_Plush::EnemyJump,
		jumpTime,
		true
	);
	*/
}

// *******************************************************************************
//                             TICK
// *******************************************************************************
// ?: Handles movement logic
void ACPP_Enemy_Plush::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Safety Check
	if (!PlayerCharRef) return;

	

	if (GetCharacterMovement()->IsMovingOnGround()) { canJump = true; }

}



// Called to bind functionality to input
void ACPP_Enemy_Plush::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

//When the enemy gets close to a wall it will inform the behevior tree to start climbing
void ACPP_Enemy_Plush::OnBeginJumpOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	APaperTileMapActor* level = Cast<APaperTileMapActor>(OtherActor);
	if (level) {
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, "Touching wall");
		if (ACPP_EnemyAiController* enemyController = Cast<ACPP_EnemyAiController>(GetController())) {
			enemyController->GetBlackboardComponent()->SetValueAsBool("CanClimb", true);
		}
		
	}
}

void ACPP_Enemy_Plush::OnEndJumpOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	APaperTileMapActor* level = Cast<APaperTileMapActor>(OtherActor);
	if (level) {
		//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, "Leaving wall");
		if (ACPP_EnemyAiController* enemyController = Cast<ACPP_EnemyAiController>(GetController())) {
			enemyController->GetBlackboardComponent()->SetValueAsBool("CanClimb", false);
		}

	}
}

void ACPP_Enemy_Plush::EnemyJump_Implementation() {
	canJump = false;
	LaunchCharacter(FVector(0, 0, GetCharacterMovement()->JumpZVelocity), false, true);
	//Jump();
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (!CMC || CMC->IsFalling()) return;

	// Play Sound
	if (Sound_Jump)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),
			Sound_Jump,
			GetActorLocation(),
			1.f,  // Volume
			FMath::FRandRange(0.8f, 1.2f) // Random Pitch
		);
	}
}




void ACPP_Enemy_Plush::OnBeginAttackOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
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

void ACPP_Enemy_Plush::OnEndAttackOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
	ACPP_PlayerChar* player = Cast<ACPP_PlayerChar>(OtherActor);
	if (player) {
		/*if (ACPP_EnemyAiController* enemyController = Cast<ACPP_EnemyAiController>(GetController())) {
			enemyController->GetBlackboardComponent()->SetValueAsBool("CanAttackPlayer", false);
		}*/
		StopAttack();
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, "Stop");
	}
}

void ACPP_Enemy_Plush::AttackPlayer_Implementation() {
	FHitResult hit;

	FVector start = GetActorLocation();
	FVector end = GetActorLocation();
	FQuat rot = FQuat(0, 0, 0, 0);
	FCollisionShape box = FCollisionShape::MakeBox(FVector(0, 50.f, 100.f));
	FCollisionQueryParams traceParams;
	traceParams.AddIgnoredActor(this);

	FCollisionObjectQueryParams objectType;
	//~~~Adds a specific object type to search, In this case it will be the pawn object type
	objectType.AddObjectTypesToQuery(ECC_Pawn);

	float knockbackDirection;

	//Checks the orientation of the sprite to determine the correct direction the attack should face
	// **NOTE: Uses the stored facing direction instead of reading sprite rotation
	// ?: This prevents incorrect attacks if another system modifies the sprite's rotation

	if (FacingDirection > 0) {

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
			player->LaunchCharacter(FVector(knockbackDirection, 0, 0), false, false);
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



void ACPP_Enemy_Plush::StopAttack() {
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, "Attack has stopped");
	if (ACPP_EnemyAiController* enemyController = Cast<ACPP_EnemyAiController>(GetController())) {
		enemyController->GetBlackboardComponent()->SetValueAsBool("CanAttackPlayer", false);
	}

}

void ACPP_Enemy_Plush::DefeatEnemy_Implementation() {
	GetSprite()->SetVisibility(false);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttackCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	//StopAttack();

	if (ACPP_EnemyAiController* enemyController = Cast<ACPP_EnemyAiController>(GetController())) {
		enemyController->GetAIPerceptionComponent()->SetSenseEnabled(UAISense_Sight::StaticClass(), false);
		enemyController->GetBlackboardComponent()->SetValueAsBool("IsDead", true);
		
	}

	bHasBeenDefeated = true;
}


//~~~Enemy Return To Patrol when lost
void ACPP_Enemy_Plush::EnemyIsLost_Implementation() {
	GetWorldTimerManager().SetTimer(ReturnToOriginTimer, this, &ACPP_Enemy_Plush::ReturnToOrigin, returnToOriginTime, false);
}

void ACPP_Enemy_Plush::ReturnToOrigin() {
	SetActorLocation(startingLocation);
	bEnemyIsLost = false;
}

// *******************************************************************************
//                                 HURT
// *******************************************************************************
void ACPP_Enemy_Plush::Hurt()
{

	// Basically, prevents hurt spam
	//if (bIsBeingHurt || amountOfLives < 1) return;
	if (bIsBeingHurt) return;

	// Decrease amount of lives
	//amountOfLives = FMath::Max(0, amountOfLives - 1);

	// ***************************
	/*           HURT           */
	// ***************************
	if (true)
		//if (amountOfLives > 0)
	{
		// Play Sound
		if (Sound_HasBeenHurt)
		{
			UGameplayStatics::PlaySoundAtLocation(GetWorld(),
				Sound_HasBeenHurt,
				GetActorLocation(),
				1.f,  // Volume
				FMath::FRandRange(0.8f, 1.2f) // Random Pitch
			);
		}

		// Used by the PaperZD Anim BP to determine whether or not to play the HURT animation
		bIsBeingHurt = true;

		// Start the timer
		GetWorld()->GetTimerManager().SetTimer(
			HurtTimer,
			this,
			&ACPP_Enemy_Plush::EndHurt,
			damageInmunityInterval,
			false
		);

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Hurt Player!"));
	}
	// ***************************
	/*           DEATH          */
	// ***************************
	else {
		//Die();
	}
}

bool ACPP_Enemy_Plush::GetEnemyCanJump() {
	return canJump;
}

/*void ACPP_Enemy_Plush::SetEnemyIsLost(bool EIL) {
	bEnemyIsLost = EIL;
}

bool ACPP_Enemy_Plush::GetEnemyIsLost() {
	return bEnemyIsLost;
}*/

