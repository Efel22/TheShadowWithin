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
class UPaperSprite;
class UPaperGroupedSpriteComponent;
class UPaperSpriteComponent;

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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTeleportShardHitEnemy, AActor*, Enemy);

/** Bookkeeping for an enemy frozen by the shard (not exposed to Blueprints). */
struct FTeleportStunInfo
{
	FTimerHandle Timer;
	float OriginalTimeDilation = 1.f;
};

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

	// ---------- Facing ----------
	/** Call this wherever the character flips its X scale, passing the same direction
	 *  value (> 0 = right, < 0 = left, 0 is ignored). Throws then follow it exactly. */
	UFUNCTION(BlueprintCallable, Category = "Teleport")
	void SetFacingDirection(float Direction);

	/** 1 = facing right, -1 = facing left. */
	UFUNCTION(BlueprintPure, Category = "Teleport")
	float GetFacingSign() const { return FacingSign; }

	UFUNCTION(BlueprintPure, Category = "Teleport")
	bool IsUnlocked() const { return bUnlocked; }

	/** Abort the aim without throwing (e.g. on damage/death). */
	UFUNCTION(BlueprintCallable, Category = "Teleport")
	void CancelAim();

	/** Full reset (call on death / respawn): cancels aiming, removes the shard without a fizzle,
	 *  ends any hit-stop and, if bClearCooldown, makes the ability ready immediately. */
	UFUNCTION(BlueprintCallable, Category = "Teleport")
	void ResetAbility(bool bClearCooldown = true);

	UFUNCTION(BlueprintPure, Category = "Teleport")
	bool IsOnCooldown() const { return State == ETeleportAbilityState::Cooldown; }

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

	/** Horizontal throw direction: world +X or -X, based on SetFacingDirection
	 *  (or the sprite's X-scale sign if that is never called). Override in BP for custom facing. */
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

	/** The shard hit an enemy (after damage/stun were applied). Use for sounds / flashes. */
	UPROPERTY(BlueprintAssignable, Category = "Teleport|Events")
	FOnTeleportShardHitEnemy OnShardHitEnemy;

	/** ResetAbility was called. */
	UPROPERTY(BlueprintAssignable, Category = "Teleport|Events")
	FOnTeleportSimpleEvent OnAbilityReset;

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

	/** Only used if SetFacingDirection is never called: tick if your sprite art faces LEFT at scale 1. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Throw")
	bool bSpriteArtFacesLeft = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Throw")
	float SpawnForwardOffset = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Throw")
	float SpawnUpOffset = 20.f;

	/** Seconds each aim pose is shown while holding. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Aim", meta = (ClampMin = "0.05"))
	float AimCycleInterval = 1.0f;

	/** true: 0-1-2-3-2-1-0...  false: 0-1-2-3-0-1... */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Aim")
	bool bPingPongAimCycle = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Cooldown", meta = (ClampMin = "0.0"))
	float CooldownDuration = 6.f;

	/** Also apply the cooldown if the projectile is lost without teleporting. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Cooldown")
	bool bCooldownOnFizzle = true;

	// ---------- Momentum ----------
	/** Share of the player's horizontal speed kept after teleporting. 0 = stop dead, 1 = keep all of it. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Momentum", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HorizontalMomentumKept = 0.5f;

	/** Share of the player's vertical speed kept (falling/jumping). 0 is usually best so teleports never "slam" you down. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Momentum", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float VerticalMomentumKept = 0.f;

	// ---------- Hit-stop (tiny pause on arrival) ----------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|HitStop")
	bool bHitStopOnArrival = true;

	/** Real-time length of the pause, in seconds. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|HitStop", meta = (ClampMin = "0.0", ClampMax = "0.5"))
	float HitStopDuration = 0.06f;

	/** Game speed during the pause (0.05 = 5% speed, almost frozen). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|HitStop", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float HitStopTimeDilation = 0.05f;

	// ---------- Enemies ----------
	/** Damage dealt to an enemy hit by the shard (sent through Unreal's damage system: Event AnyDamage). 0 = none. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Enemies", meta = (ClampMin = "0.0"))
	float EnemyDamage = 0.f;

	/** Seconds an enemy is frozen in place after being hit. 0 = no stun. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Enemies", meta = (ClampMin = "0.0"))
	float EnemyStunDuration = 1.5f;

	/** Extra gap between the character's capsule and the floor it lands on. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport")
	float TeleportSurfacePadding = 2.f;

	// ---------- Trajectory preview (dotted arc shown while aiming) ----------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Preview")
	bool bShowTrajectoryPreview = true;

	/** Small sprite used for each dot. If empty, debug dots are drawn instead (editor/dev builds only). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Preview")
	TObjectPtr<UPaperSprite> TrajectoryDotSprite;

	/** Sprite drawn where the shard will come to rest. If empty, a bigger dot is used. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Preview")
	TObjectPtr<UPaperSprite> LandingMarkerSprite;

	/** World-unit distance between dots along the arc. Raise it for fewer dots. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Preview", meta = (ClampMin = "1.0"))
	float DotSpacing = 60.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Preview", meta = (ClampMin = "0.01"))
	float DotScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Preview", meta = (ClampMin = "0.01"))
	float LandingMarkerScale = 2.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Preview")
	FLinearColor DotColor = FLinearColor::White;

	/** Dots fade from DotColor's alpha down to this alpha at the end of the arc (1 = no fade). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Preview", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EndDotAlpha = 0.35f;

	/** How many seconds of flight to predict. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Preview", meta = (ClampMin = "0.1"))
	float MaxPreviewTime = 3.f;

	/** How many wall/ceiling bounces the preview follows before stopping. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Preview", meta = (ClampMin = "0"))
	int32 MaxPreviewBounces = 3;

	/** Offset applied to every dot, e.g. toward the camera so tiles never hide them. Your camera sits on +Y. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Preview")
	FVector PreviewOffset = FVector(0.f, 10.f, 0.f);

	// ---------- Preview performance ----------
	/** Simulation step in seconds. Bigger = cheaper (fewer collision checks), slightly less precise. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Preview|Performance", meta = (ClampMin = "0.008", ClampMax = "0.1"))
	float PreviewTimeStep = 0.033f;

	/** How many times per second the arc is recalculated while aiming (it only recalculates if something changed). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Preview|Performance", meta = (ClampMin = "5.0", ClampMax = "120.0"))
	float PreviewUpdatesPerSecond = 30.f;

	/** Hard cap on dots drawn, so a very long arc can never get expensive. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Teleport|Preview|Performance", meta = (ClampMin = "2", ClampMax = "200"))
	int32 MaxPreviewDots = 40;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	void SetState(ETeleportAbilityState NewState);
	void StartAiming();
	void AdvanceAim();
	void ThrowProjectile();
	void TryTeleport();
	void StartCooldown();
	void FinishCooldown();

	/** Where the shard would spawn and how fast, for the current aim angle. Shared by the real throw and the preview. */
	bool ComputeLaunch(FVector& OutSpawnLocation, FVector& OutVelocity, FVector& OutPlaneNormal) const;

	/** bForce = recalculate even if the throw start/velocity did not change. */
	void UpdateTrajectoryPreview(bool bForce = false);
	void ClearTrajectoryPreview();
	void StopPreview();

	void StartHitStop();
	void EndHitStop();

	UFUNCTION()
	void HandleShardHitEnemy(ACPP_TeleportShard* Projectile, AActor* Enemy);

	void StunEnemy(AActor* Enemy, float Duration);
	void UnstunEnemy(TWeakObjectPtr<AActor> Enemy);

	UFUNCTION()
	void HandleProjectileArmed(ACPP_TeleportShard* Projectile);

	UFUNCTION()
	void HandleProjectileDestroyed(AActor* DestroyedActor);

	ETeleportAbilityState State = ETeleportAbilityState::Ready;
	int32 AimIndex = 0;
	int32 AimStep = 1;
	bool bIsTeleporting = false;
	float FacingSign = 1.f;
	bool bHasExplicitFacing = false;

	TWeakObjectPtr<ACPP_TeleportShard> ActiveProjectile;
	FTimerHandle AimCycleTimer;
	FTimerHandle CooldownTimer;

	UPROPERTY(Transient)
	TObjectPtr<UPaperGroupedSpriteComponent> PreviewSprites;

	UPROPERTY(Transient)
	TObjectPtr<UPaperSpriteComponent> PreviewLandingMarker;

	FTimerHandle HitStopTimer;
	bool bHitStopActive = false;
	float SavedTimeDilation = 1.f;

	TMap<TWeakObjectPtr<AActor>, FTeleportStunInfo> StunnedEnemies;

	// Preview cache: skip the simulation when nothing changed since last time.
	FVector LastPreviewStart = FVector(FLT_MAX);
	FVector LastPreviewVelocity = FVector(FLT_MAX);
	float TimeSincePreviewUpdate = 0.f;
	float TimeSincePreviewRecalc = 0.f;
};