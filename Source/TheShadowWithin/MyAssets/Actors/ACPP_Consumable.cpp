#include "CPP_Consumable.h"
#include "CPP_PlayerChar.h"
#include "PaperSpriteComponent.h"

ACPP_Consumable::ACPP_Consumable()
{
	PrimaryActorTick.bCanEverTick = false;

	// Create the sprite component
	SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));

	// Make the sprite the root component
	RootComponent = SpriteComponent;
}

void ACPP_Consumable::Collect(ACPP_PlayerChar* Player)
{
	// Make sure the player exists
	if (!Player)
	{
		return;
	}

	// Apply the consumable effect
	Player->AddHealth(RestoreAmount);

	// Remove the item after being collected
	if (bDestroyOnCollect)
	{
		Destroy();
	}
}

bool ACPP_Consumable::RequiresInput() const
{
	return bRequiresInput;
}