
#include "MyAssets/Actors/CPP_PathObstruction.h"

#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "PaperSpriteComponent.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"

#include "MyAssets/Characters/CPP_PlayerChar.h"
#include "MyAssets/Actors/ACPP_Consumable.h"

#include "EngineUtils.h"
#include "TimerManager.h"

// ********************************************************
//                       CONSTRUCTOR
// ********************************************************

ACPP_PathObstruction::ACPP_PathObstruction()
{
    PrimaryActorTick.bCanEverTick = false;

    // ROOT
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

    // SPRITE
    SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
    SpriteComponent->SetupAttachment(RootComponent);

    // PARTICLES
    ObstructionParticlesComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ObstructionParticlesComponent"));
    ObstructionParticlesComponent->SetupAttachment(RootComponent);

    // ********************************************************
    //                    BLOCKING COLLISION
    // ********************************************************

    BlockingBox = CreateDefaultSubobject<UBoxComponent>(TEXT("BlockingBox"));
    BlockingBox->SetupAttachment(RootComponent);
    BlockingBox->SetLineThickness(10.f);
    BlockingBox->SetBoxExtent(FVector(32.f, 64.f, 100.f));
    BlockingBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    BlockingBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    BlockingBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);

    // ********************************************************
    //                   ACTIVATION COLLISION
    // ********************************************************

    ActivationBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ActivationBox"));
    ActivationBox->SetupAttachment(RootComponent);
    ActivationBox->SetBoxExtent(FVector(70.f, 64.f, 100.f));
    ActivationBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ActivationBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    ActivationBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    ActivationBox->SetGenerateOverlapEvents(true);
    ActivationBox->OnComponentBeginOverlap.AddDynamic(this, &ACPP_PathObstruction::OnActivationBoxOverlap);

    // ********************************************************
    //                   PROXIMITY COLLISION
    // ********************************************************

    // Detect when the player gets close to the obstruction
    ProximityBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ProximityBox"));
    ProximityBox->SetupAttachment(RootComponent);
    ProximityBox->SetLineThickness(25.f);
    ProximityBox->SetBoxExtent(FVector(200.f, 80.f, 120.f));
    ProximityBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
    ProximityBox->SetCollisionResponseToAllChannels(ECR_Ignore);
    ProximityBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
    ProximityBox->SetGenerateOverlapEvents(true);
    ProximityBox->OnComponentBeginOverlap.AddDynamic(this, &ACPP_PathObstruction::OnProximityBoxOverlap);
}

// ********************************************************
//                        BEGIN PLAY
// ********************************************************

void ACPP_PathObstruction::BeginPlay()
{
    Super::BeginPlay();

    // Prevent activation before requirements are met
    ActivationBox->SetCollisionEnabled(
        ECollisionEnabled::NoCollision);

    // Find all consumables with the required tag
    FindRequiredConsumables();

    // Check initial requirements
    CheckRequirements();

    // Start periodic checks
    if (bAutomaticallyCheckRequirements && !bRequirementsMet)
    {
        GetWorldTimerManager().SetTimer(
            RequirementsTimerHandle,
            this,
            &ACPP_PathObstruction::CheckRequirements,
            FMath::Max(0.1f, CheckInterval),
            true
        );
    }
}

// ********************************************************
//                  FIND REQUIRED CONSUMABLES
// ********************************************************

void ACPP_PathObstruction::FindRequiredConsumables()
{
    RegionConsumables.Empty();

    if (RequiredConsumableTag.IsNone())
    {
        UE_LOG(LogTemp, Warning,
            TEXT("PathObstruction %s has no required tag!"),
            *GetName());

        return;
    }

    // Search all consumables in the loaded world
    for (TActorIterator<ACPP_Consumable> It(GetWorld()); It; ++It)
    {
        ACPP_Consumable* Consumable = *It;

        if (!IsValid(Consumable))
        {
            continue;
        }

        // Only store consumables matching the tag
        if (Consumable->ActorHasTag(RequiredConsumableTag))
        {
            RegionConsumables.Add(Consumable);
        }
    }

    UE_LOG(LogTemp, Log,
        TEXT("PathObstruction %s found %d consumables with tag %s"),
        *GetName(),
        RegionConsumables.Num(),
        *RequiredConsumableTag.ToString());
}

// ********************************************************
//                    CHECK REQUIREMENTS
// ********************************************************

void ACPP_PathObstruction::CheckRequirements()
{
    if (bRequirementsMet)
    {
        return;
    }

    if (RequiredConsumableTag.IsNone())
    {
        return;
    }

    // Prevent unlocking when no consumables were found
    if (RegionConsumables.IsEmpty())
    {
        return;
    }

    // Check whether any required consumable still exists
    for (ACPP_Consumable* Consumable : RegionConsumables)
    {
        // Destroyed consumables count as collected
        if (!IsValid(Consumable))
        {
            continue;
        }

        // This consumable has not been collected yet
        return;
    }

    // All required consumables were destroyed
    bRequirementsMet = true;

    RemoveObstruction();

    GetWorldTimerManager().ClearTimer(RequirementsTimerHandle);
}

// ********************************************************
//                    REMOVE OBSTRUCTION
// ********************************************************

void ACPP_PathObstruction::RemoveObstruction()
{
    // Disable blocking collision
    if (BlockingBox)
    {
        BlockingBox->SetCollisionEnabled(
            ECollisionEnabled::NoCollision);
    }

    // Hide obstruction sprite
    if (SpriteComponent)
    {
        SpriteComponent->SetVisibility(false);
    }

    // Stop Niagara particles
    if (ObstructionParticlesComponent)
    {
        ObstructionParticlesComponent->Deactivate();
    }

    // Disable proximity detection after the obstruction is removed
    if (ProximityBox)
    {
        ProximityBox->SetCollisionEnabled(
            ECollisionEnabled::NoCollision);
    }

    // Play Sound
    if (Sound_ObstructionRemoved)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            Sound_ObstructionRemoved,
            GetActorLocation()
        );
    }

    // Allow player detection
    if (ActivationBox)
    {
        ActivationBox->SetCollisionEnabled(
            ECollisionEnabled::QueryOnly);

        // Detect player already inside the activation box
        ActivationBox->UpdateOverlaps();
    }

    // Notify Blueprint
    OnObstructionRemoved();

    UE_LOG(LogTemp, Log,
        TEXT("PathObstruction %s removed!"),
        *GetName());
}

// ********************************************************
//                     PLAYER DETECTION
// ********************************************************

void ACPP_PathObstruction::OnActivationBoxOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    if (!bRequirementsMet || bEnemiesActivated)
    {
        return;
    }

    ACPP_PlayerChar* Player = Cast<ACPP_PlayerChar>(OtherActor);

    if (!Player)
    {
        return;
    }

    ActivateTaggedEnemies();
}

// ********************************************************
//                    PROXIMITY DETECTION
// ********************************************************

void ACPP_PathObstruction::OnProximityBoxOverlap(
    UPrimitiveComponent* OverlappedComponent,
    AActor* OtherActor,
    UPrimitiveComponent* OtherComp,
    int32 OtherBodyIndex,
    bool bFromSweep,
    const FHitResult& SweepResult
)
{
    // Only play the sound while the obstruction is active
    if (bRequirementsMet || !BlockingBox)
    {
        return;
    }

    // Make sure the blocking collision is still enabled
    if (BlockingBox->GetCollisionEnabled() == ECollisionEnabled::NoCollision)
    {
        return;
    }

    // Only detect the player
    ACPP_PlayerChar* Player = Cast<ACPP_PlayerChar>(OtherActor);

    if (!Player || Player->IsDead())
    {
        return;
    }

    // Only react to the player's capsule to avoid duplicate sounds
    if (OtherComp != Player->GetCapsuleComponent())
    {
        return;
    }

    // Play Sound
    if (Sound_PlayerNearby)
    {
        UGameplayStatics::PlaySoundAtLocation(
            GetWorld(),
            Sound_PlayerNearby,
            GetActorLocation()
        );
    }
}

// ********************************************************
//                    ACTIVATE ENEMIES
// ********************************************************

void ACPP_PathObstruction::ActivateTaggedEnemies()
{
    if (bEnemiesActivated)
    {
        return;
    }

    if (EnemyActivationTag.IsNone())
    {
        return;
    }

    bEnemiesActivated = true;

    // Find enemies matching the activation tag
    for (TActorIterator<AActor> It(GetWorld()); It; ++It)
    {
        AActor* Actor = *It;

        if (!IsValid(Actor))
        {
            continue;
        }

        if (Actor->ActorHasTag(EnemyActivationTag))
        {
            OnActivateEnemy(Actor);
        }
    }

    OnEnemiesActivated();

    // Prevent repeated activation
    if (ActivationBox)
    {
        ActivationBox->SetCollisionEnabled(
            ECollisionEnabled::NoCollision);
    }
}
