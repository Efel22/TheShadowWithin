#include "ACPP_Consumable.h"
#include "MyAssets/Characters/CPP_PlayerChar.h"
#include "PaperSpriteComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h" // Required for play_sound usage
#include "NiagaraSystem.h" // Required for niagara system usage
#include "NiagaraComponent.h" // Required for niagara component 
#include "NiagaraFunctionLibrary.h" // Required for spawning niagara systems

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

	// Niagra component setup
	ConsumableParticlesComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ConsumableParticlesComponent"));
	ConsumableParticlesComponent->SetupAttachment(RootComponent);
	ConsumableParticlesComponent->bAutoActivate = false; // ?: no arranca prendido, solo cuando se gana

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

	// Play the sound
	if (CollectSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(),
			CollectSound,
			GetActorLocation(),
			1.0f, // VOLUME
			FMath::FRandRange(0.8f, 1.2f) // PITCH
		);
	}
	
	// Play the particles
	if (CollectionParticles) UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), CollectionParticles, GetActorLocation());
}

bool ACPP_Consumable::RequiresInput() const
{
	return bRequiresInput;
}