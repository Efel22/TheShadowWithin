// CPP_TeleportShard.h
// Throwable "ender pearl" style actor. It becomes ARMED only after touching a surface.
//  - Walls / ceilings: reflects off the surface (angle in = angle out), then keeps flying (armed).
//  - Floors: stops dead on contact (armed).
// The player (not this actor) decides when to teleport - see UTeleportAbilityComponent.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPP_TeleportShard.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class ACPP_TeleportShard;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeleportProjectileArmed, ACPP_TeleportShard*, Projectile);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeleportProjectileImpact, const FHitResult&, Hit, bool, bBouncedOffSurface);

UCLASS(Blueprintable)
class ACPP_TeleportShard : public AActor
{
	GENERATED_BODY()

public:
	ACPP_TeleportShard();

	/** Called by the ability component right after spawning (before FinishSpawning). */
	void Launch(const FVector& LaunchVelocity, const FVector& PlaneNormal);

	UFUNCTION(BlueprintPure, Category = "Teleport")
	bool IsArmed() const { return bArmed; }

	/** True once it has come to rest on a floor. */
	UFUNCTION(BlueprintPure, Category = "Teleport")
	bool HasLanded() const { return bLanded; }

	/** Normal of the floor it landed on (Up if it has not landed). */
	UFUNCTION(BlueprintPure, Category = "Teleport")
	FVector GetLandingNormal() const { return LandingNormal; }

	UFUNCTION(BlueprintPure, Category = "Teleport")
	float GetCollisionRadius() const;

	UFUNCTION(BlueprintPure, Category = "Teleport")
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

	/** Fired the first time it touches any surface. */
	UPROPERTY(BlueprintAssignable, Category = "Teleport")
	FOnTeleportProjectileArmed OnArmed;

	/** Fired on every surface contact (bounce or landing). */
	UPROPERTY(BlueprintAssignable, Category = "Teleport")
	FOnTeleportProjectileImpact OnImpact;

protected:
	virtual void BeginPlay() override;

	// ---- Blueprint hooks for VFX / SFX in your BP child ----
	UFUNCTION(BlueprintImplementableEvent, Category = "Teleport", meta = (DisplayName = "On Armed"))
	void BP_OnArmed();

	UFUNCTION(BlueprintImplementableEvent, Category = "Teleport", meta = (DisplayName = "On Impact"))
	void BP_OnImpact(const FHitResult& Hit, bool bBouncedOffSurface);

	// ---- Components ----
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	// ---- Tuning ----
	/** Surface normal Z >= this counts as a floor (0.7 ~= 45 degree slope). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Bounce", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float FloorNormalThreshold = 0.7f;

	/** Speed kept after a wall/ceiling bounce. 1 = no loss. The reflection ANGLE is always preserved. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Bounce", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float WallBounceSpeedMultiplier = 0.7f;

	/** Destroy (fizzle) if it never touches anything within this time. 0 = never. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Lifetime", meta = (ClampMin = "0.0"))
	float UnarmedLifetime = 10.f;

	/** Destroy (fizzle) this long after being armed if the player never teleports. 0 = stay forever. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Lifetime", meta = (ClampMin = "0.0"))
	float ArmedLifetime = 0.f;

	/** When it lands on a Movable component (moving platform), ride along with it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport")
	bool bAttachToMovingSurfaces = true;

private:
	UFUNCTION()
	void HandleBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	UFUNCTION()
	void HandleStop(const FHitResult& ImpactResult);

	void Arm();

	bool bArmed = false;
	bool bLanded = false;
	FVector LandingNormal = FVector::UpVector;
};