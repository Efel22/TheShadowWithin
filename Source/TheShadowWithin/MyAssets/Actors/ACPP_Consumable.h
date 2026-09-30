#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyAssets/Interfaces/CollectableInterface.h"
#include "ACPP_Consumable.generated.h"

class UPaperSpriteComponent;
class ACPP_PlayerChar;
class USphereComponent;
class UNiagaraSystem; // Required for playing niagara systems
class UNiagaraComponent; // Required for niagara particle components
class USoundBase; // Required for sound base

UCLASS()
class THESHADOWWITHIN_API ACPP_Consumable 
	: public AActor, public ICollectableInterface
{
	GENERATED_BODY()

public:

	ACPP_Consumable();

	// Called when the player collects this item
	virtual void Collect(ACPP_PlayerChar* Player) override;

	// Returns whether this item requires the player
	// to press the interact key
	virtual bool RequiresInput() const override;

protected:

	// Sprite displayed in the world
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components | Visual")
	UPaperSpriteComponent* SpriteComponent;

	// Attached particles
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components | Visual")
	UNiagaraComponent* ConsumableParticlesComponent;

	// Sound that plays when the item is collected
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components | Audio")
	USoundBase* CollectSound;

	// Particles that play when the item is collected
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components | Audio")
	UNiagaraSystem* CollectionParticles;

	// Area used to detect when the player gets close enough
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USphereComponent* CollectionSphereComponent;

	// If true, the player must press the interact key
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectable")
	bool bRequiresInput = false;

	// Amount of health restored
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable")
	float RestoreAmount = 25.0f;

	// If true, destroy the consumable after collection
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collectable")
	bool bDestroyOnCollect = true;

	// Called when something enters the collection sphere
	UFUNCTION()
	void OnCollectionSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};