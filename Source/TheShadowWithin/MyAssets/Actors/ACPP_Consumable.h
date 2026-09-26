#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CollectableInterface.h"
#include "CPP_Consumable.generated.h"

class UPaperSpriteComponent;
class ACPP_PlayerChar;

UCLASS()
class THESHADOWWITHIN_API ACPP_Consumable : public AActor, public ICollectableInterface
{
	GENERATED_BODY()

public:
	ACPP_Consumable();

	// Called when the player collects this item
	virtual void Collect(ACPP_PlayerChar* Player) override;

	// Returns whether this item requires the player to press the interact key
	bool RequiresInput() const;

protected:

	// Sprite displayed in the world
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPaperSpriteComponent* SpriteComponent;

	// If true, the player must press the interact key to collect this item
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectable")
	bool bRequiresInput = false;

	// Amount restored by this consumable
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable")
	float RestoreAmount = 25.0f;

	// If true, destroy the consumable after it is collected
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectable")
	bool bDestroyOnCollect = true;
};