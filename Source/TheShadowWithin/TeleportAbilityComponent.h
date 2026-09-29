// TeleportAbilityComponent.h
// Add to your player character. Drives the whole throw -> arm -> teleport -> cooldown loop.
//
// Flow (all on one input, Tab):
//   Ready    --press-->   Aiming (angle auto-cycles through ThrowAnglesDegrees)
//   Aiming   --release--> InFlight (projectile thrown at the angle showing)
//   InFlight --projectile touches a surface--> Armed
//   Armed    --press-->   teleport, projectile destroyed, Cooldown
//   Cooldown --CooldownDuration--> Ready
// If the projectile is destroyed without teleporting (fell out of world / lifetime) it "fizzles".

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TeleportAbilityComponent.generated.h"

class ACPP_TeleportShard;

UENUM(BlueprintType)
enum class ETeleportAbilityState : uint8
{
	Ready,
	Aiming,
	InFlight,   // thrown, has not touched a surface yet - cannot teleport
	Armed,      // touched a surface - press again to teleport
	Cooldown
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeleportStateChanged, ETeleportAbilityState, NewState, ETeleportAbilityState, OldState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeleportAimIndexChanged, int32, AimIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeleportThrown, int32, AimIndex, ACPP_TeleportShard*, Projectile);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTeleported, FVector, FromLocation, FVector, ToLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTeleportSimpleEvent);

UCLASS(ClassGroup = (Abilities), Blueprintable, meta = (BlueprintSpawnableComponent))
class UTeleportAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UTeleportAbilityComponent();

	// ---------- Input (call from your character Blueprint) ----------
	/** Enhanced Input "Started" on the Teleport action. */
	UFUNCTION(BlueprintCallable, Category = "Teleport|Input")
	void OnTeleportInputPressed();

	/** Enhanced Input "Completed" on the Teleport action. */
	UFUNCTION(BlueprintCallable, Category = "Teleport|Input")
	void OnTeleportInputReleased();

	// ---------- Unlock ----------
	UFUNCTION(BlueprintCallable, Category = "Teleport")
	void SetUnlocked(bool bNewUnlocked);

	UFUNCTION(BlueprintCallable, Category = "Teleport")
	void UnlockAbility() { SetUnlocked(true); }

	UFUNCTION(BlueprintPure, Category = "Teleport")
	bool IsUnlocked() const { return bUnlocked; }

	/** Abort the aim without throwing (e.g. on damage/death). */
	UFUNCTION(BlueprintCallable, Category = "Teleport")
	void CancelAim();

	// ---------- Queries (use these in your PaperZD AnimBP / HUD) ----------
	UFUNCTION(BlueprintPure, Category = "Teleport")
	ETeleportAbilityState GetState() const { return State; }

	UFUNCTION(BlueprintPure, Category = "Teleport")
	bool IsAiming() const { return State == ETeleportAbilityState::Aiming; }

	/** 0..NumAngles-1, which aim pose/angle is currently showing. */
	UFUNCTION(BlueprintPure, Category = "Teleport")
	int32 GetAimIndex() const { return AimIndex; }

	UFUNCTION(BlueprintPure, Category = "Teleport")
	float GetCurrentThrowAngle() const;

	UFUNCTION(BlueprintPure, Category = "Teleport")
	bool CanTeleport() const { return State == ETeleportAbilityState::Armed; }

	UFUNCTION(BlueprintPure, Category = "Teleport")
	float GetCooldownRemaining() const;

	/** 1 = cooldown just started, 0 = ready. Handy for a radial HUD fill. */
	UFUNCTION(BlueprintPure, Category = "Teleport")
	float GetCooldownFractionRemaining() const;

	UFUNCTION(BlueprintPure, Category = "Teleport")
	ACPP_TeleportShard* GetActiveProjectile() const { return ActiveProjectile.Get(); }

	/** Horizontal direction the character faces. Default = actor forward with Z removed.
	 *  Override in BP if you flip the sprite with scale instead of rotating the actor. */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Teleport")
	FVector GetThrowForwardDirection() const;

	// ---------- Events ----------
	UPROPERTY(BlueprintAssignable, Category = "Teleport|Events")
	FOnTeleportStateChanged OnStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Teleport|Events")
	FOnTeleportSimpleEvent OnAimStarted;

	/** Fires on aim start and every time the auto-cycle switches pose. */
	UPROPERTY(BlueprintAssignable, Category = "Teleport|Events")
	FOnTeleportAimIndexChanged OnAimIndexChanged;

	UPROPERTY(BlueprintAssignable, Category = "Teleport|Events")
	FOnTeleportSimpleEvent OnAimCancelled;

	/** Play your throw animation here. */
	UPROPERTY(BlueprintAssignable, Category = "Teleport|Events")
	FOnTeleportThrown OnThrown;

	/** The projectile touched a surface - teleport is now available. */
	UPROPERTY(BlueprintAssignable, Category = "Teleport|Events")
	FOnTeleportSimpleEvent OnTeleportArmed;

	UPROPERTY(BlueprintAssignable, Category = "Teleport|Events")
	FOnTeleported OnTeleported;

	/** Armed, but no valid spot to place the character. Projectile is kept. */
	UPROPERTY(BlueprintAssignable, Category = "Teleport|Events")
	FOnTeleportSimpleEvent OnTeleportFailed;

	/** Input pressed while in flight / on cooldown / locked (play a "nope" sound). */
	UPROPERTY(BlueprintAssignable, Category = "Teleport|Events")
	FOnTeleportSimpleEvent OnTeleportDenied;

	/** Projectile was destroyed without being used. */
	UPROPERTY(BlueprintAssignable, Category = "Teleport|Events")
	FOnTeleportSimpleEvent OnProjectileFizzled;

	UPROPERTY(BlueprintAssignable, Category = "Teleport|Events")
	FOnTeleportSimpleEvent OnCooldownFinished;

	// ---------- Settings ----------
	/** Locked until the player unlocks it. Tick this in the character defaults while testing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teleport")
	bool bUnlocked = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport")
	TSubclassOf<ACPP_TeleportShard> ProjectileClass;

	/** Hard-coded throw angles in degrees above horizontal, in the order the aim cycles.
	 *  Index N here should match aim animation N. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Throw")
	TArray<float> ThrowAnglesDegrees = { 10.f, 30.f, 50.f, 70.f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Throw", meta = (ClampMin = "0.0"))
	float ThrowSpeed = 1200.f;

	/** Add the character's current velocity to the throw (off = angles are exact). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Throw")
	bool bAddOwnerVelocity = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Throw")
	float SpawnForwardOffset = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Throw")
	float SpawnUpOffset = 20.f;

	/** Seconds each aim pose is shown while holding. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Aim", meta = (ClampMin = "0.05"))
	float AimCycleInterval = 0.35f;

	/** true: 0-1-2-3-2-1-0...  false: 0-1-2-3-0-1... */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Aim")
	bool bPingPongAimCycle = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Cooldown", meta = (ClampMin = "0.0"))
	float CooldownDuration = 6.f;

	/** Also apply the cooldown if the projectile is lost without teleporting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Cooldown")
	bool bCooldownOnFizzle = true;

	/** Kill the character's momentum on arrival (ender pearl style). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport")
	bool bResetVelocityOnTeleport = true;

	/** Extra gap between the character's capsule and the floor it lands on. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport")
	float TeleportSurfacePadding = 2.f;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void SetState(ETeleportAbilityState NewState);
	void StartAiming();
	void AdvanceAim();
	void ThrowProjectile();
	void TryTeleport();
	void StartCooldown();
	void FinishCooldown();

	UFUNCTION()
	void HandleProjectileArmed(ACPP_TeleportShard* Projectile);

	UFUNCTION()
	void HandleProjectileDestroyed(AActor* DestroyedActor);

	ETeleportAbilityState State = ETeleportAbilityState::Ready;
	int32 AimIndex = 0;
	int32 AimStep = 1;
	bool bIsTeleporting = false;

	TWeakObjectPtr<ACPP_TeleportShard> ActiveProjectile;
	FTimerHandle AimCycleTimer;
	FTimerHandle CooldownTimer;
};