#include "ACPP_Consumable.h"
#include "MyAssets/Characters/CPP_PlayerChar.h"
#include "PaperSpriteComponent.h"
#include "Components/SphereComponent.h"

// *******************************************************************************
//                             CONSTRUCTOR
// *******************************************************************************
ACPP_Consumable::ACPP_Consumable()
{
	PrimaryActorTick.bCanEverTick = false;

	// ROOT
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// Create the sprite component
	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
	SpriteComponent->SetupAttachment(RootComponent);

	// Create the sphere component
	CollectionSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollectionSphereComponent"));
	CollectionSphereComponent->SetupAttachment(RootComponent);
	CollectionSphereComponent->SetSphereRadius(100.0f);

	// Enable overlap detection
	CollectionSphereComponent->SetGenerateOverlapEvents(true);

	// Call our function when something enters the sphere
	CollectionSphereComponent->OnComponentBeginOverlap.AddDynamic(
		this,
		&ACPP_Consumable::OnCollectionSphereOverlap
	);

}

// *******************************************************************************
//                             SPHERE DETECTION
// *******************************************************************************
void ACPP_Consumable::OnCollectionSphereOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	// Check if the actor entering the sphere is the player
	ACPP_PlayerChar* Player = Cast<ACPP_PlayerChar>(OtherActor);

	if (!Player)
	{
		return;
	}

	// Automatically collect if this item does NOT require input
	if (!bRequiresInput)
	{
		Collect(Player);
	}
}

// *******************************************************************************
//                             COLLECT FUNCTIONALITY
// *******************************************************************************
void ACPP_Consumable::Collect(ACPP_PlayerChar* Player)
{
	// Make sure the player exists
	if (!Player)
	{
		return;
	}

	// Restore player health (This funciton doesn't exist yet)
	//Player->AddHealth(RestoreAmount);

	// Destroy this consumable if necessary
	if (bDestroyOnCollect)
	{
		Destroy();
	}
}

bool ACPP_Consumable::RequiresInput() const
{
	return bRequiresInput;
}