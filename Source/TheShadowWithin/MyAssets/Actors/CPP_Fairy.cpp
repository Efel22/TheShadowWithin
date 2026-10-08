// Fill out your copyright notice in the Description page of Project Settings.


#include "CPP_Fairy.h"
#include "PaperFlipbookComponent.h" // Required for flipbook component
#include "NiagaraSystem.h" // Required for niagara system usage
#include "NiagaraComponent.h" // Required for niagara component
#include "NiagaraFunctionLibrary.h" // Required for spawning niagara systems
#include "MyAssets/Characters/CPP_PlayerChar.h" // Player ref
#include "MyAssets/Actors/ACPP_Consumable.h" // Consumable ref
#include "Kismet/GameplayStatics.h" // Required for player and actor finding
#include "TimerManager.h" // Required for timers


// Sets default values
ACPP_Fairy::ACPP_Fairy()
{
	// Set this actor to call Tick() every frame.
	PrimaryActorTick.bCanEverTick = true;

	// ROOT
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Setup the fairy's flipbook comp.
	FairyFlipbookComp = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("Fairy Flipbook"));
	FairyFlipbookComp->SetupAttachment(RootComponent);
	FairyFlipbookComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Disable fairy collision

	// Setup the fairy's ongoing particles
	Fairy_OnGoingParticlesComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Fairy Ongoing Particles"));
	Fairy_OnGoingParticlesComp->SetupAttachment(FairyFlipbookComp);
}


// Called when the game starts or when spawned
void ACPP_Fairy::BeginPlay()
{
	Super::BeginPlay();

	// Get the player character
	Player = Cast<ACPP_PlayerChar>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)
	);

	// Safety check
	if (!Player)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("FAIRY: Player could NOT be found!"));

		return;
	}

	// Lock the fairy to the player's Y position
	// ?: The fairy should NEVER move in the Y axis
	LockedY = Player->GetActorLocation().Y;

	// Make sure the fairy starts in the correct Y position
	FVector FairyLocation = GetActorLocation();
	FairyLocation.Y = LockedY;
	SetActorLocation(FairyLocation);

	// Start the timer that searches for the nearest consumable
	GetWorld()->GetTimerManager().SetTimer(
		SearchTimer,
		this,
		&ACPP_Fairy::FindNearestConsumable,
		SearchInterval,
		true
	);

	// Start the timer that guides the player towards the nearest consumable
	GetWorld()->GetTimerManager().SetTimer(
		GuidanceTimer,
		this,
		&ACPP_Fairy::GuidePlayerToConsumable,
		GuidanceInterval,
		true
	);

	// Find the first nearest consumable immediately
	FindNearestConsumable();
}


// Called every frame
void ACPP_Fairy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Safety check
	if (!Player) return;

	switch (FairyState)
	{
		// Follow and fly around the player
	case EFairyState::Following:

		FollowPlayer(DeltaTime);

		break;

		// Fly towards the nearest consumable
	case EFairyState::Guiding:

		FlyToConsumable(DeltaTime);

		break;

		// Return to the player
	case EFairyState::Returning:

		FollowPlayer(DeltaTime);

		break;

	default:
		break;
	}
}


// *******************************************************************************
//                                  MOVEMENT
// *******************************************************************************
void ACPP_Fairy::FollowPlayer(float DeltaTime)
{
	// Safety check
	if (!Player) return;

	// Get the current game time
	float CurrentTime = GetWorld()->GetTimeSeconds();

	// Calculate the fairy's flying position around the player
	// ?: Cos controls LEFT / RIGHT (X)
	// ?: Sin controls UP / DOWN (Z)
	float OffsetX = FMath::Cos(CurrentTime * OrbitSpeed) * OrbitRadius;
	float OffsetZ = FMath::Sin(CurrentTime * OrbitSpeed) * OrbitRadius;

	// Get the player's current position
	FVector PlayerLocation = Player->GetActorLocation();

	// Create the fairy's desired position
	// ?: Y is ALWAYS LockedY since the fairy should NEVER move in the Y axis
	FVector TargetLocation = FVector(
		PlayerLocation.X + OffsetX,
		LockedY,
		PlayerLocation.Z + OffsetZ
	);

	// Store the fairy's current position
	FVector CurrentLocation = GetActorLocation();

	// Smoothly move the fairy towards its target position
	FVector NewLocation = FMath::VInterpTo(
		CurrentLocation,
		TargetLocation,
		DeltaTime,
		FollowSpeed
	);

	// Extra safety
	// ?: NEVER allow the fairy to move in the Y axis
	NewLocation.Y = LockedY;

	// Get the direction the fairy is moving in the X axis
	float DirectionX = NewLocation.X - CurrentLocation.X;

	// Flip the fairy based on its movement direction
	FlipFairy(DirectionX);

	// Move the fairy
	SetActorLocation(NewLocation);

	// Check if the fairy has returned to the player
	if (FairyState == EFairyState::Returning)
	{
		// Get the distance between the fairy and its following position
		float DistanceX = TargetLocation.X - NewLocation.X;
		float DistanceZ = TargetLocation.Z - NewLocation.Z;

		float DistanceToFollowPosition = FMath::Sqrt(
			DistanceX * DistanceX +
			DistanceZ * DistanceZ
		);

		// Check if the fairy is close enough to the player again
		if (DistanceToFollowPosition <= ReturnDistance)
		{
			// Return to normal following behavior
			FairyState = EFairyState::Following;

			// DEBUG
			if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green, TEXT("FAIRY: Returned to FOLLOWING!"));
		}
	}
}
 
// *******************************************************************************
//                                FLIP FAIRY
// *******************************************************************************
// ?: Flips the fairy's sprite based on the direction its flying
void ACPP_Fairy::FlipFairy(float DirectionX)
{
	// Safety check
	if (!FairyFlipbookComp) return;

	// Don't change direction if the fairy is only moving vertically
	if (FMath::IsNearlyZero(DirectionX)) return;

	// Get the fairy's current scale
	FVector FixedScale = FairyFlipbookComp->GetRelativeScale3D();

	// Keep the original scale size and only change its direction
	FixedScale.X = FMath::Abs(FixedScale.X) * FMath::Sign(DirectionX);

	// Flip the fairy
	FairyFlipbookComp->SetRelativeScale3D(FixedScale);
}


// *******************************************************************************
//                           FIND NEAREST CONSUMABLE
// *******************************************************************************
// ?: Finds the nearest consumable within the specified SearchRadius
void ACPP_Fairy::FindNearestConsumable()
{
	// Safety check
	if (!Player) return;

	// Stores all consumables currently in the world
	TArray<AActor*> Consumables;

	// Find all consumables
	UGameplayStatics::GetAllActorsOfClass(
		GetWorld(),
		ACPP_Consumable::StaticClass(),
		Consumables
	);

	// Reset the nearest consumable
	NearestConsumable = nullptr;

	// Store the maximum allowed distance (squared)
	float MaxDistanceSquared = FMath::Square(SearchRadius);

	// Stores the shortest distance found
	float NearestDistance = MaxDistanceSquared;

	// Get the player's current position
	FVector PlayerLocation = Player->GetActorLocation();

	// Ignore the Y axis
	PlayerLocation.Y = 0.f;

	// Check every consumable
	for (AActor* ConsumableActor : Consumables)
	{
		// Cast actor to consumable
		ACPP_Consumable* Consumable = Cast<ACPP_Consumable>(ConsumableActor);

		// Safety check
		if (!Consumable) continue;

		// Get the consumable's position
		FVector ConsumableLocation = Consumable->GetActorLocation();

		// Ignore the Y axis
		ConsumableLocation.Y = 0.f;

		// Calculate the squared distance between the PLAYER and the consumable
		float Distance = FVector::DistSquared(
			PlayerLocation,
			ConsumableLocation
		);

		// Ignore consumables outside the SearchRadius
		if (Distance > MaxDistanceSquared) continue;

		// Check if this consumable is closer than the previous one
		if (Distance <= NearestDistance)
		{
			NearestDistance = Distance;
			NearestConsumable = Consumable;
		}
	}
}


// *******************************************************************************
//                             CONSUMABLE GUIDANCE
// *******************************************************************************
void ACPP_Fairy::GuidePlayerToConsumable()
{
	// Safety check
	if (!NearestConsumable) return;

	// Don't start another guidance if the fairy is already guiding
	if (FairyState != EFairyState::Following) return;

	// Change the fairy's state
	FairyState = EFairyState::Guiding;

	// Get the fairy and consumable positions
	FVector FairyLocation = GetActorLocation();
	FVector ConsumableLocation = NearestConsumable->GetActorLocation();

	// Calculate the direction from the fairy towards the consumable
	FVector Direction = ConsumableLocation - FairyLocation;

	// Ignore the Y axis
	Direction.Y = 0.f;

	// Safety check
	if (Direction.IsNearlyZero()) return;

	Direction.Normalize();

	// Get the rotation that points towards the consumable
	FRotator GuidanceRotation = Direction.Rotation();

	// Spawn the guidance particles
	if (Fairy_GuidanceParticles)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			Fairy_GuidanceParticles,
			FairyLocation,
			GuidanceRotation
		);
	}

	// Play guidance sound
	if (Fairy_GuidanceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			Fairy_GuidanceSound,
			FairyLocation,
			1.f,
			FMath::FRandRange(0.9f, 1.1f)
		);
	}
}

// *******************************************************************************
//                           FLY TO CONSUMABLE
// *******************************************************************************
// ?: Makes the fairy fly towards the nearest consumable
void ACPP_Fairy::FlyToConsumable(float DeltaTime)
{
	// Safety check
	// ?: If the consumable no longer exists, return to the player
	if (!NearestConsumable)
	{
		FairyState = EFairyState::Returning;
		return;
	}

	// Store the fairy's current position
	FVector CurrentLocation = GetActorLocation();

	// Get the consumable's position
	FVector TargetLocation = NearestConsumable->GetActorLocation();

	// Lock the target's Y position to the fairy's Y position
	// ?: The fairy should NEVER move in the Y axis
	TargetLocation.Y = LockedY;

	// Smoothly move the fairy towards the consumable
	FVector NewLocation = FMath::VInterpTo(
		CurrentLocation,
		TargetLocation,
		DeltaTime,
		GuidanceFlySpeed
	);

	// Extra safety
	// ?: NEVER allow the fairy to move in the Y axis
	NewLocation.Y = LockedY;

	// Get the direction the fairy is moving in the X axis
	float DirectionX = NewLocation.X - CurrentLocation.X;

	// Flip the fairy based on its movement direction
	FlipFairy(DirectionX);

	// Move the fairy
	SetActorLocation(NewLocation);

	// Get the X and Z distance between the fairy and consumable
	float DistanceX = TargetLocation.X - NewLocation.X;
	float DistanceZ = TargetLocation.Z - NewLocation.Z;

	// Calculate the fairy's distance from the consumable
	float DistanceToConsumable = FMath::Sqrt(
		DistanceX * DistanceX +
		DistanceZ * DistanceZ
	);

	// Check if the fairy has reached the consumable
	if (DistanceToConsumable <= ConsumableReachDistance)
	{
		// Change the fairy's state so it doesn't continue flying towards the consumable
		FairyState = EFairyState::Returning;

		// Wait before returning to the player
		GetWorld()->GetTimerManager().SetTimer(
			ReturnTimer,
			this,
			&ACPP_Fairy::ReturnToPlayer,
			GuidanceWaitTime,
			false
		);
	}
}


// *******************************************************************************
//                              RETURN TO PLAYER
// *******************************************************************************
// ?: Makes the fairy return to following the player
void ACPP_Fairy::ReturnToPlayer()
{
	FairyState = EFairyState::Returning;
}