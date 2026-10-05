// Fill out your copyright notice in the Description page of Project Settings.
//Legend, Use ctrl + f to easily find code blocks utilizing key words:
/*
Movement Functions
*/

#include "MyAssets/Characters/CPP_PlayerChar.h"
#include "PaperFlipbookComponent.h" // Required for sprite flipping
#include "Camera/CameraComponent.h"
#include "Engine/Engine.h" // Used to print strings
#include "MyAssets/Characters/CPP_PlayerController.h" // Required for Vine Player Sprite Facing (In DoMove() )
#include "Kismet/GameplayStatics.h"                   // ^
#include "Components/ArrowComponent.h" // Used to determine where the "center" of the vine is (used in player's sprite flipping)
#include "GameFramework/CharacterMovementComponent.h" // Used to disable the player's gravity (using the MOVEMENT_MOVE)
#include "TeleportAbilityComponent.h" // Teleport throw direction (adjust the path if the component lives in a subfolder, e.g. "MyAssets/Abilities/TeleportAbilityComponent.h")
#include "MyAssets/Characters/ACPP_Enemy_Basic.h"

ACPP_PlayerChar::ACPP_PlayerChar() {
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(RootComponent);
	Camera->SetRelativeLocationAndRotation(FVector(0.0f, 1920.0f, 0.0f), FRotator(0.0f, -90.0f, 0.0f));
	Camera->SetProjectionMode(ECameraProjectionMode::Orthographic);
	Camera->SetAutoCalculateOrthoPlanes(false);
	Camera->SetOrthoWidth(orthoWidth);

	// Center of Vine Component
	CenterOfPlayerComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Center of Player"));
	CenterOfPlayerComponent->SetupAttachment(RootComponent);
}


// Called when the game starts or when spawned
void ACPP_PlayerChar::BeginPlay()
{
	Super::BeginPlay();

	SetRespawnPoint(GetActorLocation());
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~Basic Movement
void ACPP_PlayerChar::DoMove(float Forward) {
	const FVector MoveDir = FVector(1.0f, Forward > 0.0f ? 0.1f : -0.1f, 0.0f);
	AddMovementInput(MoveDir, Forward);

	ACPP_PlayerController* PC = Cast<ACPP_PlayerController>(
		UGameplayStatics::GetPlayerController(GetWorld(), 0)
	);
	/*if (!PC) {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("- PlayerController is null!"));
		return;
	}*/

	/*if (!PC->GetIsHoldingVine())
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Purple, TEXT("- PlayerController is HOLDING VINE = FALSE!"));
	}
	else {
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 1.f, FColor::Purple, TEXT("- PlayerController is HOLDING VINE = TRUE!"));
	}*/

	// Flip the player's sprite based on the movement direction
	// **NOTE: Doesn't work if the player is holding onto a vine!
	// **NOTE: (!PC || ...) prevents a crash if the PlayerController is ever null
	if (Forward != 0.0f && (!PC || !PC->GetIsHoldingVine()))
	{
		FVector fixedScale = GetSprite()->GetRelativeScale3D();

		// Keep the original scale size and only change its direction
		fixedScale.X = FMath::Abs(fixedScale.X) * FMath::Sign(Forward);

		GetSprite()->SetRelativeScale3D(fixedScale);

		// Keep the teleport throw direction in sync with the sprite
		if (UTeleportAbilityComponent* Teleport = FindComponentByClass<UTeleportAbilityComponent>())
		{
			Teleport->SetFacingDirection(Forward);
		}
	}

	// Make the player face the vine regardless 
	if (PC && PC->GetIsHoldingVine()) FaceVine();;
}

void ACPP_PlayerChar::DoClimbVine(float Forward) {
	AddMovementInput(FVector::UpVector, Forward);
	FaceVine();
}

void ACPP_PlayerChar::DoJump() {
	Jump();
}

void ACPP_PlayerChar::DoStopJump() {
	if (GetCharacterMovement()->Velocity.Z > 0) {
		GetCharacterMovement()->Velocity.Z *= jumpCutOff;
	}
	StopJumping();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// *******************************************************************************
//                             REMOVE DARKNESS
// *******************************************************************************
// ?: Executes death/defeat logic
void ACPP_PlayerChar::Die()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("PLAYER HAS BEEN UNALIVED!!! :O"));
}

// *******************************************************************************
//                             STARTCLIMBINGVINE
// *******************************************************************************
void ACPP_PlayerChar::StartClimbingVine()
{
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	CMC->SetMovementMode(MOVE_Flying);
	CMC->Velocity = FVector::ZeroVector;
}

// *******************************************************************************
//                             STOPCLIMBINGVINE
// *******************************************************************************
void ACPP_PlayerChar::StopClimbingVine()
{
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	CMC->SetMovementMode(MOVE_Falling);
}

// *******************************************************************************
//                             STOP HORIZONTAL MOVEMENT
// *******************************************************************************
// ?: STOPS horizontal sliding when on a vine
void ACPP_PlayerChar::StopHorizontalMovement()
{
	GetCharacterMovement()->Velocity.X = 0.0f;
}

// *******************************************************************************
//                             STOP VERTICAL MOVEMENT
// *******************************************************************************
// ?: STOPS _vertical_ sliding when on a vine
//    Called from the player controller, required a boolean though
void ACPP_PlayerChar::StopVerticalMovement()
{
	GetCharacterMovement()->Velocity.Z = 0.0f;
}

// *******************************************************************************
//                             FACE VINE
// *******************************************************************************
// ?: Overrides the Scale logic (the one that changes where the player's sprite is
//    visually looking at) and makes it so it faces the vine its climbing on instead
void ACPP_PlayerChar::FaceVine()
{
	FVector FixedScale = GetSprite()->GetRelativeScale3D();

	float PlayerX = CenterOfPlayerComponent->GetComponentLocation().X;

	if (currentVine_PosX > PlayerX)
	{
		// Vine is to the RIGHT of player
		FixedScale.X = FMath::Abs(FixedScale.X);
	}
	else
	{
		// Vine is to the LEFT of player
		FixedScale.X = -FMath::Abs(FixedScale.X);
	}

	GetSprite()->SetRelativeScale3D(FixedScale);

	// Keep the teleport throw direction in sync with the sprite
	if (UTeleportAbilityComponent* Teleport = FindComponentByClass<UTeleportAbilityComponent>())
	{
		Teleport->SetFacingDirection(FixedScale.X);   // + = right, - = left
	}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//Simple attacks
void ACPP_PlayerChar::DoSwordAttack() {
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, "Throwing hands");
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

	//Checks the orientation of the sprite to determine the correct direction the attack should face
	if (GetSprite()->GetRelativeScale3D().X > 0) {
		end.X = GetActorLocation().X + swordHitBoxSize;
	}
	else {
		end.X = GetActorLocation().X + (-swordHitBoxSize);
	}

	if (GetWorld()->SweepSingleByObjectType(hit, start, end, rot, objectType, box, traceParams)) {
		
		AACPP_Enemy_Basic* enemy = Cast<AACPP_Enemy_Basic>(hit.GetActor());
		if (enemy) {
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, "Enemy hit");
		}
	}

}