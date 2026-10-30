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
#include "Kismet/GameplayStatics.h"       
#include "NiagaraFunctionLibrary.h" // Required for spawning niagara systems            // ^
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

	// Store the reference to the player controller
	PlayerController = Cast<ACPP_PlayerController>(GetController());


	// Initial Respawn point should be at player start
	SetRespawnPoint(GetActorLocation());
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~Basic Movement
void ACPP_PlayerChar::DoMove(float Forward) {

	// DEAD? DO NOTHING
	if (IsDead()) return;

	const FVector MoveDir = FVector(1.0f, Forward > 0.0f ? 0.1f : -0.1f, 0.0f);
	AddMovementInput(MoveDir, Forward);

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
	//if (Forward != 0.0f && (!PC || !PC->GetIsHoldingVine()))
	//{
	//	FVector fixedScale = GetSprite()->GetRelativeScale3D();

	//	// Keep the original scale size and only change its direction
	//	fixedScale.X = FMath::Abs(fixedScale.X) * FMath::Sign(Forward);

	//	GetSprite()->SetRelativeScale3D(fixedScale);

	//	// Keep the teleport throw direction in sync with the sprite
	//	if (UTeleportAbilityComponent* Teleport = FindComponentByClass<UTeleportAbilityComponent>())
	//	{
	//		Teleport->SetFacingDirection(Forward);
	//	}
	//}

	// Flip the player's sprite based on movement direction
	if (Forward != 0.0f && (!PlayerController || !PlayerController->GetIsHoldingVine()))
	{
		// Rotate sprite instead of using negative scale
		float RotationZ = Forward > 0.0f ? 0.0f : -180.0f;

		GetSprite()->SetRelativeRotation(FRotator(0.0f, RotationZ, 0.0f));

		// Keep teleport throw direction in sync
		if (UTeleportAbilityComponent* Teleport =
			FindComponentByClass<UTeleportAbilityComponent>())
		{
			Teleport->SetFacingDirection(Forward);
		}
	}

	// Make the player face the vine regardless 
	if (PlayerController && PlayerController->GetIsHoldingVine()) FaceVine();;
}

void ACPP_PlayerChar::DoClimbVine(float Forward) {
	AddMovementInput(FVector::UpVector, Forward);
	FaceVine();
}

void ACPP_PlayerChar::DoJump()
{
	// DEAD? DO NOTHING
	if (IsDead()) return;

	UCharacterMovementComponent* CMC = GetCharacterMovement();
	if (!CMC) return;

	// Check if the player is holding onto a vine
	if (PlayerController && PlayerController->GetIsHoldingVine())
	{
		// Release the vine
		PlayerController->SetIsHoldingVine(false);

		// Restore gravity
		StopClimbingVine();

		// Launch the player upward
		LaunchCharacter(FVector(0.0f, 0.0f, 600.0f), false, true);
	}
	else
	{
		// Prevent jumping while already in the air
		if (CMC->IsFalling()) return;

		// Normal jump
		Jump();
	}

	// Play Sound
	if (Sound_Jump)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			Sound_Jump,
			GetActorLocation(),
			1.f, // Volume
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
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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

	// Remove any thrown shard and make the teleport ready again
	if (UTeleportAbilityComponent* Teleport = FindComponentByClass<UTeleportAbilityComponent>())
	{
		Teleport->ResetAbility(true);
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

		// Restore normal movement after respawning
		CMC->SetMovementMode(MOVE_Walking);
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

	// Make sure the player is no longer holding a vine
	if (PC)
	{
		// Player is no holding vine
		PC->SetIsHoldingVine(false);

		// Make the mouse cursor visible
		PC->bShowMouseCursor = false;

		// Set the player's input to UI only
		FInputModeGameOnly InputMode;

		PC->SetInputMode(InputMode);
	}

	// Remove any thrown shard and make the teleport ready again
	if (UTeleportAbilityComponent* Teleport = FindComponentByClass<UTeleportAbilityComponent>())
	{
		Teleport->ResetAbility(true);
	}
}

// *******************************************************************************
//                             STARTCLIMBINGVINE
// *******************************************************************************
void ACPP_PlayerChar::StartClimbingVine()
{
	// Player cannot grab a vine if being hurt
	if (
		bIsBeingHurt ||
		bIsDefending 
	) 
	return;

	// Play Particles
	if (VineInteractParticles)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),
			VineInteractParticles,
			CenterOfPlayerComponent->GetComponentLocation()
		);
	}

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

	// Play Particles
	if (VineInteractParticles)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(),
			VineInteractParticles,
			CenterOfPlayerComponent->GetComponentLocation()
		);
	}
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

	// Determine whether the vine is left or right
	float FacingDirection = currentVine_PosX > PlayerX ? 1.0f : -1.0f;

	// Rotate sprite around Z
	float RotationZ = FacingDirection > 0.0f ? 0.0f : -180.0f;

	GetSprite()->SetRelativeRotation(FRotator(0.0f, RotationZ, 0.0f));

	// Keep teleport throw direction in sync
	if (UTeleportAbilityComponent* Teleport =
		FindComponentByClass<UTeleportAbilityComponent>())
	{
		Teleport->SetFacingDirection(FacingDirection);
	}

	//if (currentVine_PosX > PlayerX)
	//{
	//	// Vine is to the RIGHT of player
	//	FixedScale.X = FMath::Abs(FixedScale.X);
	//}
	//else
	//{
	//	// Vine is to the LEFT of player
	//	FixedScale.X = -FMath::Abs(FixedScale.X);
	//}

	//GetSprite()->SetRelativeScale3D(FixedScale);

	//// Keep the teleport throw direction in sync with the sprite
	//if (UTeleportAbilityComponent* Teleport = FindComponentByClass<UTeleportAbilityComponent>())
	//{
	//	Teleport->SetFacingDirection(FixedScale.X);   // + = right, - = left
	//}
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//Simple attacks
void ACPP_PlayerChar::DoSwordAttack() {

	if (!PlayerController) return;

	if (bIsSwordAttacking || 
		bIsBeingHurt || 
		bIsBeingHealed ||
		PlayerController->GetIsHoldingVine()
	) return;

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, "Throwing hands");
	
	HitBoxTrace(swordHitBoxSize);
	if (enemy) {
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, "Enemy hit");
		enemy->Defeat();
	}

	// Start the timer
	GetWorld()->GetTimerManager().SetTimer(
		AttackTimer,
		this,
		&ACPP_PlayerChar::EndSwordAttack,
		attackCooldownInterval,
		false
	);

	// Play Sound
	if (Sound_SwordAttack)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),
			Sound_SwordAttack,
			GetActorLocation(),
			1.f,  // Volume
			FMath::FRandRange(0.8f, 1.2f) // Random Pitch
		);
	}

	bIsSwordAttacking = true;
}

void ACPP_PlayerChar::DoSwordParry() {

	if (bIsDefending) return;

	//prevents the player from getting damage
	bIsDefending = true;

	// Play Sound
	if (Sound_Parry)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),
			Sound_Parry,
			GetActorLocation(),
			1.f,  // Volume
			FMath::FRandRange(0.8f, 1.2f) // Random Pitch
		);
	}

	HitBoxTrace(swordDefenseBoxSize);
	//float parryKnockbackDirection;
	/*if (GetSprite()->GetRelativeScale3D().X > 0) {
		
		parryKnockbackDirection = parryKnockback;
	}
	else {
		
		parryKnockbackDirection = -(parryKnockback);
	}*/

	float RotationZ = GetSprite()->GetRelativeRotation().Yaw;

	float parryKnockbackDirection =
		FMath::IsNearlyZero(RotationZ) ? parryKnockback : -parryKnockback;

	if (enemy) {
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, "Parried attack");
		enemy->LaunchCharacter(FVector(parryKnockbackDirection, 0, 0), true, false);
	}

}

void ACPP_PlayerChar::DoSwordDefense() {
	//prevents the player from getting damage
	bIsDefending = true;


	// Play Sound
	if (Sound_Parry)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),
			Sound_Parry,
			GetActorLocation(),
			1.f,  // Volume
			FMath::FRandRange(0.8f, 1.2f) // Random Pitch
		);
	}

	HitBoxTrace(swordDefenseBoxSize);
	if (enemy) {
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, "Attack Defended");
		//enemy->LaunchCharacter(FVector(parryKnockback, 0, 0), true, false);
	}
}

void ACPP_PlayerChar::DoStopSwordDefense() {
	bIsDefending = false;
}

void ACPP_PlayerChar::HitBoxTrace(float hitBoxSize) {
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
	//if (GetSprite()->GetRelativeScale3D().X > 0) {
	//	end.X = GetActorLocation().X + hitBoxSize;
	//	
	//}
	//else {
	//	end.X = GetActorLocation().X + (-hitBoxSize);
	//	
	//}

	float RotationZ = GetSprite()->GetRelativeRotation().Yaw;

	if (FMath::IsNearlyZero(RotationZ))
	{
		// Facing right
		end.X = GetActorLocation().X + hitBoxSize;
	}
	else
	{
		// Facing left
		end.X = GetActorLocation().X - hitBoxSize;
	}

	if (GetWorld()->SweepSingleByObjectType(hit, start, end, rot, objectType, box, traceParams)) {

		enemy = Cast<AACPP_Enemy_Basic>(hit.GetActor());
		
	}
}

// *******************************************************************************
//                                 HURT
// *******************************************************************************
void ACPP_PlayerChar::Hurt()
{

	// Basically, prevents hurt spam
	if (bIsBeingHurt || amountOfLives < 1) return;

	if (!PlayerController) return;

	// If during a hurt event, the player was holding onto a vine, let go of it
	// If the player is holding onto a vine, release it and restore gravity
	if (PlayerController->GetIsHoldingVine())
	{
		PlayerController->SetIsHoldingVine(false);
		StopClimbingVine();
	}
	
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


//Setters and Getters
bool ACPP_PlayerChar::GetIsPlayerDefending() {
	return bIsDefending;
}