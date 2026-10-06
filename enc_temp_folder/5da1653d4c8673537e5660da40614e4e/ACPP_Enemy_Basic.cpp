// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAssets/Characters/ACPP_Enemy_Basic.h"
#include "Kismet/GameplayStatics.h" // Required for getting the player character
#include "MyAssets/Characters/CPP_PlayerChar.h" // Required for player char. casting
#include "GameFramework/CharacterMovementComponent.h" // Required for changing the character's movement speed (FOR NOW)
#include "Components/SphereComponent.h"

// *******************************************************************************
//                             CONSTRUCTOR
// *******************************************************************************
// ?: Looks pretty :D
AACPP_Enemy_Basic::AACPP_Enemy_Basic()
{
	AttackCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Attack Collider"));
	AttackCollider->SetupAttachment(RootComponent);
	AttackCollider->OnComponentBeginOverlap.AddDynamic(this, &AACPP_Enemy_Basic::OnBeginOverlap);

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// *******************************************************************************
//                             BEGIN PLAY
// *******************************************************************************
// ?: Assigns references, movement speed, and originalScale
void AACPP_Enemy_Basic::BeginPlay()
{
	Super::BeginPlay();

	// Get the reference to the player's character
	PlayerCharRef = Cast<ACPP_PlayerChar>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	// Set the movement speed 
	// (*FOR NOW* ILL BE USING THIS SO ALL PROPERTIES ARE FOUND IN THE SAME TAB INSIDE THE EDITOR,)
	GetCharacterMovement()->MaxWalkSpeed = FMovementSpeed;

	// Store the original scale
	originalScale = GetActorScale();

	GetWorldTimerManager().SetTimer(JumpTimer, this, &AACPP_Enemy_Basic::EnemyJump, jumpTime, true);
}

// *******************************************************************************
//                             TICK
// *******************************************************************************
// ?: Handles movement logic
void AACPP_Enemy_Basic::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Safety Check
	if (!PlayerCharRef) return;


	// How far is it from the player?
	float distance = FVector::Dist(PlayerCharRef->GetActorLocation(), GetActorLocation());

	// Is it within range?
	if (distance <= FDetectionRange)
	{
		// Where should this enemy face?
		// Based on the two characters positions (player and this enemy's) get its sign as it determines the location
		// NOTE: This is based on the asumption that 'X' axis represents "right" and "left"!!!!
		int direction = FMath::Sign(
			PlayerCharRef->GetActorLocation().X - GetActorLocation().X
		);

		// Prevents enemy from disappearing visually
		if (direction != 0)
		{
			// Sets the scale based on the direction
			SetActorScale3D(FVector(originalScale.X * direction, originalScale.Y, originalScale.Z));
		}
		

		// Moves the enemy in the 'x' axis in the direction that faces the player
		AddMovementInput(FVector(1.f, 0.f, 0.f), direction);

		/*int verticalDirection = FMath::Sign(
			PlayerCharRef->GetActorLocation().Z - GetActorLocation().Z
		);

		if (verticalDirection > 0) {
			Jump();
		}*/

	}

}

// Called to bind functionality to input
void AACPP_Enemy_Basic::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

//After set amount of time passes the enemy jumps based on is the player has the higher ground
void AACPP_Enemy_Basic::EnemyJump() {
	int verticalDirection = FMath::Sign(
		PlayerCharRef->GetActorLocation().Z - GetActorLocation().Z
	);

	if (verticalDirection > 0) {
		Jump();
	}
}

void AACPP_Enemy_Basic::OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) {
	ACPP_PlayerChar* player = Cast<ACPP_PlayerChar>(OtherActor);
	if (player) {
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AACPP_Enemy_Basic::AttackPlayer, attackTime, false);
		bEnemyIsAttacking = true;
	}
}

void AACPP_Enemy_Basic::AttackPlayer() {
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
	if (GetActorRelativeScale3D().X > 0) {
		end.X = GetActorLocation().X + attackHitBox;
		knockbackDirection = knockback;
	}
	else {
		end.X = GetActorLocation().X + (-attackHitBox);
		knockbackDirection = -(knockback);
	}

	if (GetWorld()->SweepSingleByObjectType(hit, start, end, rot, objectType, box, traceParams)) {

		ACPP_PlayerChar* player = Cast<ACPP_PlayerChar>(hit.GetActor());
		if (player) {
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, "Attacking Player");
			player->LaunchCharacter(FVector(knockbackDirection, 0, 0), true, false);
			bEnemyIsAttacking = false;
			
		}
		else {
			return;
		}

		GetWorldTimerManager().SetTimer(AttackCooldownTimer, this, &AACPP_Enemy_Basic::AttackPlayer, attackCooldownTime, false);
	}
}