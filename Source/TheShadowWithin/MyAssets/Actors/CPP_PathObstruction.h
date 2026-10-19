
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPP_PathObstruction.generated.h"

class UBoxComponent;
class UPaperSpriteComponent;
class UNiagaraComponent;
class ACPP_PlayerChar;
class ACPP_Consumable;

UCLASS()
class THESHADOWWITHIN_API ACPP_PathObstruction : public AActor
{
    GENERATED_BODY()

public:
    ACPP_PathObstruction();

    UFUNCTION(BlueprintCallable, Category = "Path Obstruction")
    void CheckRequirements();

protected:
    virtual void BeginPlay() override;

    // ********************************************************
    //                       COMPONENTS
    // ********************************************************

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* BlockingBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UBoxComponent* ActivationBox;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components | Visual")
    UPaperSpriteComponent* SpriteComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components | Visual")
    UNiagaraComponent* ObstructionParticlesComponent;

    // ********************************************************
    //                       REQUIREMENTS
    // ********************************************************

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Obstruction | Requirements")
    FName RequiredConsumableTag = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Obstruction | Enemies")
    FName EnemyActivationTag = NAME_None;

    // Automatically populated during BeginPlay
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Path Obstruction | Requirements")
    TArray<TObjectPtr<ACPP_Consumable>> RegionConsumables;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Obstruction | Requirements")
    bool bAutomaticallyCheckRequirements = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Path Obstruction | Requirements", meta = (ClampMin = "0.1"))
    float CheckInterval = 0.25f;

    // ********************************************************
    //                          STATE
    // ********************************************************

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path Obstruction | State")
    bool bRequirementsMet = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Path Obstruction | State")
    bool bEnemiesActivated = false;

    // ********************************************************
    //                        FUNCTIONS
    // ********************************************************

    // Finds all consumables matching RequiredConsumableTag
    void FindRequiredConsumables();

    // Removes the obstruction
    void RemoveObstruction();

    // Activates enemies with EnemyActivationTag
    void ActivateTaggedEnemies();

    UFUNCTION()
    void OnActivationBoxOverlap(
        UPrimitiveComponent* OverlappedComponent,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult
    );

    UFUNCTION(BlueprintImplementableEvent, Category = "Path Obstruction | Enemies")
    void OnActivateEnemy(AActor* Enemy);

    UFUNCTION(BlueprintImplementableEvent, Category = "Path Obstruction | Events")
    void OnObstructionRemoved();

    UFUNCTION(BlueprintImplementableEvent, Category = "Path Obstruction | Events")
    void OnEnemiesActivated();

private:
    FTimerHandle RequirementsTimerHandle;
};
