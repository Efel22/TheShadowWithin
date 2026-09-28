#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "CollectableInterface.generated.h"

class ACPP_PlayerChar;

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
	virtual void Collect(ACPP_PlayerChar* Player) = 0;

	// Determines whether the collectable requires input
	virtual bool RequiresInput() const = 0;
};