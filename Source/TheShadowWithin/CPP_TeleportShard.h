
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
class UPaperSpriteComponent;
class UNiagaraComponent;
class UNiagaraSystem;
class USoundBase;
class ACPP_TeleportShard;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeleportProjectileArmed, ACPP_TeleportShard*, Projectile);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeleportProjectileImpact, const FHitResult&, Hit, bool, bBouncedOffSurface);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeleportProjectileHitEnemy, ACPP_TeleportShard*, Projectile, AActor*, Enemy);

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

	UFUNCTION(BlueprintPure, Category = "Teleport")
	USphereComponent* GetCollisionComponent() const { return Collision; }

	// Used by the trajectory preview so it bounces/lands exactly like the real shard.
	float GetFloorNormalThreshold() const { return FloorNormalThreshold; }
	float GetWallBounceSpeedMultiplier() const { return WallBounceSpeedMultiplier; }

	/** Fired the first time it touches any surface. */
	UPROPERTY(BlueprintAssignable, Category = "Teleport")
	FOnTeleportProjectileArmed OnArmed;

	/** Fired on every surface contact (bounce or landing). */
	UPROPERTY(BlueprintAssignable, Category = "Teleport")
	FOnTeleportProjectileImpact OnImpact;

	/** Fired when the shard hits an enemy (any pawn except the thrower, filtered by EnemyTag). */
	UPROPERTY(BlueprintAssignable, Category = "Teleport")
	FOnTeleportProjectileHitEnemy OnHitEnemy;

	// Get the sounds and particles used by the teleport ability component
	USoundBase* GetThrowSound() const { return Sound_Throw; }
	USoundBase* GetAimChangedSound() const { return Sound_AimChanged; }
	USoundBase* GetTeleportSound() const { return Sound_Teleport; }
	UNiagaraSystem* GetTeleportParticles() const { return TeleportParticles; }

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	// ---- Blueprint hooks for VFX / SFX in your BP child ----
	UFUNCTION(BlueprintImplementableEvent, Category = "Teleport", meta = (DisplayName = "On Armed"))
	void BP_OnArmed();

	UFUNCTION(BlueprintImplementableEvent, Category = "Teleport", meta = (DisplayName = "On Impact"))
	void BP_OnImpact(const FHitResult& Hit, bool bBouncedOffSurface);

	UFUNCTION(BlueprintImplementableEvent, Category = "Teleport", meta = (DisplayName = "On Hit Enemy"))
	void BP_OnHitEnemy(AActor* Enemy);

	// ---- Components ----
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	// Sprite displayed for the teleport shard
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UPaperSpriteComponent> SpriteComponent;

	// Niagara component that follows the shard while it is flying
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> FlyingParticlesComponent;

	// Niagara component that plays while the shard is resting on the ground
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> LandedParticlesComponent;

	// ***************************************************************************************************************
	//                                                  SOUNDS
	// ***************************************************************************************************************

	// Sound that plays when the shard is thrown
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	USoundBase* Sound_Throw = nullptr;

	// Sound that plays when the aiming angle changes
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	USoundBase* Sound_AimChanged = nullptr;

	// Sound that plays when the shard hits a wall or ceiling
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	USoundBase* Sound_Impact = nullptr;

	// Sound that plays when the shard lands on the ground
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	USoundBase* Sound_Landing = nullptr;

	// Sound that plays when the shard hits an enemy
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	USoundBase* Sound_HitEnemy = nullptr;

	// Sound that plays when the player uses the shard to teleport
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sounds")
	USoundBase* Sound_Teleport = nullptr;

	// ***************************************************************************************************************
	//                                                  PARTICLES
	// ***************************************************************************************************************

	// Particles that continuously follow the shard while it is flying
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Particles")
	UNiagaraSystem* FlyingParticles = nullptr;

	// Particles that continuously appear when the shard has landed
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Particles")
	UNiagaraSystem* LandedParticles = nullptr;

	// Particles that play whenever the shard collides with something
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Particles")
	UNiagaraSystem* ImpactParticles = nullptr;

	// Particles that play when the shard is used to teleport
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Particles")
	UNiagaraSystem* TeleportParticles = nullptr;

	// ***************************************************************************************************************
	//                                                  SPRITE ROTATION
	// ***************************************************************************************************************

	// Should the sprite tilt slightly while flying?
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport|Visual")
	bool bTiltWhileFlying = true;

	// Maximum angle of the sprite's tilt
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport|Visual")
	float TiltAmount = 12.f;

	// Speed of the tilting movement
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport|Visual")
	float TiltSpeed = 5.f;

	// Original pitch of the sprite so it can be restored after landing
	float OriginalSpritePitch = 0.f;

	// Time used to calculate the sprite's tilting movement
	float TiltTime = 0.f;

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

	// ---- Enemies ----
	/** Hit enemies while flying (bounce off them and notify the ability component). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Enemies")
	bool bInteractWithEnemies = true;

	/** Only pawns with this Actor Tag count as enemies. Leave as None to treat every pawn (except the thrower) as an enemy. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Enemies")
	FName EnemyTag = NAME_None;

	/** Speed kept after bouncing off an enemy. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Enemies", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EnemyBounceSpeedMultiplier = 0.5f;

	/** Hitting an enemy arms the shard (teleport becomes available). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Enemies")
	bool bArmOnEnemyHit = true;

private:
	UFUNCTION()
	void HandleBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

	UFUNCTION()
	void HandleStop(const FHitResult& ImpactResult);

	UFUNCTION()
	void HandleBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	/** 2D plane the shard is locked to (zero if not constrained). */
	FVector PlaneNormalUsed = FVector::ZeroVector;

	/** Enemies already hit this throw (each enemy is hit at most once). */
	TArray<TWeakObjectPtr<AActor>> HitEnemies;

	void Arm();

	// Play an impact sound and spawn particles at the collision location
	void PlayImpactEffects(const FVector& ImpactLocation, USoundBase* Sound);

	// Stop the flying particles and restore the sprite's original rotation
	void StopFlyingEffects();

	bool bArmed = false;
	bool bLanded = false;
	FVector LandingNormal = FVector::UpVector;
};
