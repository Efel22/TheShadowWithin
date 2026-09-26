#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CollectableInterface.generated.h"

UINTERFACE(MinimalAPI)
class UCollectableInterface : public UInterface
{
	GENERATED_BODY()
};

class THESHADOWWITHIN_API ICollectableInterface
{
	GENERATED_BODY()

public:

	// Called when something collects this actor
	virtual void Collect(class ACPP_PlayerChar* Player) = 0;

	// Determines whether the collectable requieres input or not
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectable")
	bool bRequiresInput = false;
};