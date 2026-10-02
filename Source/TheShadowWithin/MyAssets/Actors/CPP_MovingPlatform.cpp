// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAssets/Actors/CPP_MovingPlatform.h"
#include "Components/BoxComponent.h" 
#include "Components/InterpToMovementComponent.h"
#include "PaperSpriteComponent.h"

// Sets default values
ACPP_MovingPlatform::ACPP_MovingPlatform()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// ROOT
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Setup the platform sprite comp.
	PlatformSpriteComp = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("Platform Sprite"));
	PlatformSpriteComp->SetupAttachment(RootComponent); 
	PlatformSpriteComp->SetCollisionEnabled(ECollisionEnabled::NoCollision); // Disable sprite collison

	// Setup the collison box comp. 
	// ?: Blocks ONLY PAWN collisions
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Box"));
	CollisionBox->SetupAttachment(RootComponent);
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); // Remove all other collisions 
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block); // Blocks PAWN collisions (player and enemies)

	InterpMovComp = CreateDefaultSubobject<UInterpToMovementComponent>(TEXT("Interp Mov. Comp."));
}

