// Fill out your copyright notice in the Description page of Project Settings.
//Legend, Use ctrl + f to easily find code blocks utilizing key words:
/*
Movement Functions
*/

#include "MyAssets/Characters/CPP_PlayerChar.h"
#include "PaperZDCharacter.h"
#include "PaperFlipbookComponent.h" // Required for sprite flipping
#include "Camera/CameraComponent.h"
#include "Blueprint/UserWidget.h" // Required to show specified widget from header file
#include "Engine/Engine.h" // Used to print strings
#include "MyAssets/Characters/CPP_PlayerController.h" // Required for Vine Player Sprite Facing (In DoMove() )
#include "Kismet/GameplayStatics.h"                   // ^
#include "Components/ArrowComponent.h" // Used to determine where the "center" of the vine is (used in player's sprite flipping)
#include "GameFramework/CharacterMovementComponent.h" // Used to disable the player's gravity (using the MOVEMENT_MOVE)
#include "TeleportAbilityComponent.h" // Teleport throw direction (adjust the path if the component lives in a subfolder, e.g. "MyAssets/Abilities/TeleportAbilityComponent.h")

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

	// Initial Respawn point should be at player start
	SetRespawnPoint(GetActorLocation());
}

void ACPP_PlayerChar::DoMove(float Forward) {

	// DEAD? DO NOTHING
	if (IsDead()) return;

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
	// DEAD? DO NOTHING
	if (IsDead()) return;

	Jump();

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

void ACPP_PlayerChar::DoStopJump() {
	if (GetCharacterMovement()->Velocity.Z > 0) {
		GetCharacterMovement()->Velocity.Z *= jumpCutOff;
	}
	StopJumping();
}

// *******************************************************************************
//                                      DIE
// *******************************************************************************
// ?: Executes death/defeat logic
void ACPP_PlayerChar::Die()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("PLAYER HAS BEEN UNALIVED!!! :O"));

	// Insert Death Logic here?

	// Play Sound
	if (Sound_HasBeenDefeated)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),
			Sound_HasBeenDefeated,
			GetActorLocation(),
			1.f,  // Volume
			FMath::FRandRange(0.8f, 1.2f) // Random Pitch
		);
	}
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Death Player!"));

	// Player is alive
	isDead = true;

	// Make sure a Win Widget was assigned in the Editor
	if (!DeathWidgetClass) return;

	// Create the assigned Win Widget
	UUserWidget* DeathWidget = CreateWidget<UUserWidget>(
		GetWorld(),
		DeathWidgetClass
	);

	// Make sure the Widget was successfully created
	if (!DeathWidget) return;

	// Add the Death Widget to the player's screen
	DeathWidget->AddToViewport();

	// Get the player's controller
	ACPP_PlayerController* PC = Cast<ACPP_PlayerController>(
		UGameplayStatics::GetPlayerController(GetWorld(), 0)
	);

	if (PC)
	{
		// Make the mouse cursor visible
		PC->bShowMouseCursor = true;

		// Set the player's input to UI only
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(DeathWidget->TakeWidget());

		PC->SetInputMode(InputMode);
	}
}

// *******************************************************************************
//                                      RESPAWN
// *******************************************************************************
// ?: Executes death/defeat logic
void ACPP_PlayerChar::Respawn()
{
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("PLAYER has respawned!!! :O"));

	// Play Sound
	if (Sound_HasRespawned)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),
			Sound_HasRespawned,
			GetActorLocation(),
			1.f,  // Volume
			FMath::FRandRange(0.8f, 1.2f) // Random Pitch
		);
	}

	// Stop previous movement
	if (UCharacterMovementComponent* CMC = GetCharacterMovement())
	{
		CMC->StopMovementImmediately();
	}

	// Move player to respawn point
	SetActorLocation(GetRespawnPoint());

	// Set the amount of lives to max
	amountOfLives = maxAmountOfLives;

	// Set it to false
	isDead = false;

	// Get the player's controller
	ACPP_PlayerController* PC = Cast<ACPP_PlayerController>(
		UGameplayStatics::GetPlayerController(GetWorld(), 0)
	);

	if (PC)
	{
		// Make the mouse cursor visible
		PC->bShowMouseCursor = false;

		// Set the player's input to UI only
		FInputModeGameOnly InputMode;

		PC->SetInputMode(InputMode);
	}
}

// *******************************************************************************
//                             STARTCLIMBINGVINE
// *******************************************************************************
void ACPP_PlayerChar::StartClimbingVine()
{
	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (!CMC) return;
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

// *******************************************************************************
//                                 HURT
// *******************************************************************************
void ACPP_PlayerChar::Hurt()
{

	// Basically, prevents hurt spam
	if (bIsBeingHurt || amountOfLives < 1) return;

	// Decrease amount of lives
	amountOfLives = FMath::Max(0, amountOfLives - 1);

	// Call Event Dispatcher
	OnLivesChanged.Broadcast(amountOfLives);

	// ***************************
	/*           HURT           */ 
	// ***************************
	if (amountOfLives > 0)
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
			&ACPP_PlayerChar::EndHurt,
			damageInmunityInterval,
			false
		);

		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("Hurt Player!"));
	}
	// ***************************
	/*           DEATH          */
	// ***************************
	else {
		Die();
	}
}

// *******************************************************************************
//                                 END HURT
// *******************************************************************************
void ACPP_PlayerChar::EndHurt()
{
	// Used by the PaperZD Anim BP to determine whether or not to play the HURT animation
	bIsBeingHurt = false;
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("* * * RESET HURT!"));
}

// *******************************************************************************
//                                 HEAL
// *******************************************************************************
void ACPP_PlayerChar::Heal()
{

	// Basically, prevents heal spam
	if (bIsBeingHealed) return;

	// Increate amount of lives
	amountOfLives = FMath::Min(maxAmountOfLives, amountOfLives + 1 );

	// Call Event Dispatcher
	OnLivesChanged.Broadcast(amountOfLives);

	// Used by the PaperZD Anim BP to determine whether or not to play the HEAL animation
	bIsBeingHealed = true;

	// Play Sound
	if (Sound_HasBeenHealed)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),
			Sound_HasBeenHealed,
			GetActorLocation(),
			1.f,  // Volume
			FMath::FRandRange(0.8f, 1.2f) // Random Pitch
		);
	}

	// Start the timer
	GetWorld()->GetTimerManager().SetTimer(
		HealTimer,
		this,
		&ACPP_PlayerChar::EndHeal,
		healInmunityInterval,
		false
	);
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("Healed Player!"));
}

// *******************************************************************************
//                                 END HEAL
// *******************************************************************************
void ACPP_PlayerChar::EndHeal()
{
	// Used by the PaperZD Anim BP to determine whether or not to play the HEAL animation
	bIsBeingHealed = false;
	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("* * * RESET HEALED!"));
}