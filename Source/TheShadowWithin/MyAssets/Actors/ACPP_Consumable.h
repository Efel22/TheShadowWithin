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

// Defines the behavior of this consumable actor
UENUM(BlueprintType)
enum class EConsumableType : uint8
{
	ActivatesTeleportation, // Activates the player's ability to teleport
	RemovesTeleportation,   // Removes the player's ability to teleport
	Generic                 // Does nothing
};

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

	// Called every frame
	virtual void Tick(float DeltaTime) override;
protected:

	// Called when the game starts
	virtual void BeginPlay() override;

	// Defines behavior by the consumable whenever its collected 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components | Behavior")
	EConsumableType ConsumableType = EConsumableType::Generic; // Does nothing by default 

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

	// *******************************************************************************
	//                             DARKNESS
	// *******************************************************************************

	// If true, adds darkness. Otherwise, removes darkness.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable | Darkness")
	bool bAddsDarkness = false;

	// Amount of darkness added or removed when collected
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable | Darkness", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float DarknessAmount = 0.1f;

	// *******************************************************************************
	//                             FLOATING ANIMATION
	// *******************************************************************************

	// Determines whether the sprite bobs up and down
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable | Floating")
	bool bShouldBob = false;

	// Maximum distance the sprite moves above and below its original position
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable | Floating",
		meta = (EditCondition = "bShouldBob", ClampMin = "0.0"))
	float BobHeight = 15.0f;

	// Number of complete bobbing cycles per second
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consumable | Floating", meta = (EditCondition = "bShouldBob", ClampMin = "0.0"))
	float BobFrequency = 0.5f;

	// Stores the sprite's original relative position
	FVector OriginalSpriteLocation = FVector::ZeroVector;

	// Tracks how long the sprite has been floating
	float BobTime = 0.0f;

	// Updates the sprite's floating animation
	void UpdateBobbing(float DeltaTime);

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