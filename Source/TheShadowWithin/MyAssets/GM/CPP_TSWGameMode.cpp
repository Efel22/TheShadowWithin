
#include "MyAssets/GM/CPP_TSWGameMode.h"
#include "Materials/MaterialInstanceDynamic.h" // Needed for the darkness post process material
#include "Materials/MaterialInterface.h" // Needed for the darkness post process material
#include "MyAssets/Characters/CPP_PlayerChar.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"

// *******************************************************************************
//                             CONSTRUCTOR
// *******************************************************************************
ACPP_TSWGameMode::ACPP_TSWGameMode()
{
	// Tick is only needed while the tint is transitioning
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

// *******************************************************************************
//                             BEGIN PLAY
// *******************************************************************************
void ACPP_TSWGameMode::BeginPlay()
{
	Super::BeginPlay();

	// Make sure a darkness material was assigned
	if (!DarknessMaterial)
	{
		return;
	}

	// Create a dynamic version of the darkness material
	DarknessMaterialDynamic = UMaterialInstanceDynamic::Create(
		DarknessMaterial,
		this
	);

	// Make sure the dynamic material was successfully created
	if (!DarknessMaterialDynamic)
	{
		return;
	}

	// Set its initial darkness value
	DarknessMaterialDynamic->SetScalarParameterValue(
		TEXT("Darkness"),
		Darkness
	);

	// Set the initial tint values
	TintStrength = FMath::Clamp(TintStrength, 0.0f, 1.0f);
	UpdateTintMaterial();

	// Get a reference to the player's character
	ACPP_PlayerChar* PlayerChar = Cast<ACPP_PlayerChar>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)
	);

	// Make sure the player and camera exist
	if (!IsValid(PlayerChar) || !PlayerChar->GetCamera())
	{
		return;
	}

	// Apply the post process material to the player's camera
	ApplyPostProcessToCamera(PlayerChar->GetCamera());

	// Start the darkness increment timer (its on a loop)
	StartDarknessTimer();
}

// *******************************************************************************
//                             TICK
// *******************************************************************************
void ACPP_TSWGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Update the tint only while a transition is active
	if (bIsTintTransitioning)
	{
		UpdateTintTransition(DeltaTime);
	}
}

// *******************************************************************************
//                             START DARKNESS
// *******************************************************************************
void ACPP_TSWGameMode::StartDarknessTimer()
{
	if (DoesDarknessIncrements)
	{
		// Prevent starting multiple timers
		GetWorldTimerManager().ClearTimer(DarknessTimer);

		GetWorldTimerManager().SetTimer(
			DarknessTimer,
			[this]()
			{
				AddDarkness(DarknessIncrements);
			},
			FMath::Max(0.01f, SecondsBetweenIncrements),
			true
		);
	}
}

// *******************************************************************************
//                             ADD DARKNESS
// *******************************************************************************
void ACPP_TSWGameMode::AddDarkness(float Amount)
{
	// Increase the darkness
	Darkness += Amount;

	// Keep the darkness between 0 and 1
	Darkness = FMath::Clamp(
		Darkness,
		0.0f,
		1.0f
	);

	// Update the darkness material
	if (DarknessMaterialDynamic)
	{
		DarknessMaterialDynamic->SetScalarParameterValue(
			TEXT("Darkness"),
			Darkness
		);
	}

	// Check if the player has reached maximum darkness
	if (Darkness >= 1.0f && !bDarknessDeathTriggered)
	{
		ACPP_PlayerChar* PlayerRef = Cast<ACPP_PlayerChar>(
			UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)
		);

		// Make sure the player exists and is not already dead
		if (IsValid(PlayerRef) && !PlayerRef->IsDead())
		{
			bDarknessDeathTriggered = true;

			PlayerRef->Die();
		}
	}
}

// *******************************************************************************
//                             REMOVE DARKNESS
// *******************************************************************************
void ACPP_TSWGameMode::RemoveDarkness(float Amount)
{
	// Decrease the darkness
	Darkness -= Amount;

	// Keep the darkness between 0 and 1
	Darkness = FMath::Clamp(
		Darkness,
		0.0f,
		1.0f
	);

	// Update the darkness material
	if (DarknessMaterialDynamic)
	{
		DarknessMaterialDynamic->SetScalarParameterValue(
			TEXT("Darkness"),
			Darkness
		);
	}

	// Allow the death check again after darkness has decreased
	if (Darkness < 1.0f)
	{
		bDarknessDeathTriggered = false;
	}
}

// *******************************************************************************
//                             SET TINT
// *******************************************************************************
void ACPP_TSWGameMode::SetTint(
	FLinearColor NewColor,
	float NewStrength,
	float TransitionDuration
)
{
	// Store the target tint values
	TargetTintColor = NewColor;

	TargetTintStrength = FMath::Clamp(
		NewStrength,
		0.0f,
		1.0f
	);

	// Store the current tint values
	// ?: A new transition can interrupt an existing one
	InitialTintColor = TintColor;
	InitialTintStrength = TintStrength;

	// Set the transition duration
	TintTransitionDuration = FMath::Max(
		0.0f,
		TransitionDuration
	);

	// Reset elapsed time
	TintTransitionElapsed = 0.0f;

	// Apply the tint instantly if the duration is zero
	if (TintTransitionDuration <= 0.0f)
	{
		TintColor = TargetTintColor;
		TintStrength = TargetTintStrength;

		bIsTintTransitioning = false;

		UpdateTintMaterial();

		SetActorTickEnabled(false);

		return;
	}

	// Skip the transition if the values are already reached
	if (TintColor.Equals(TargetTintColor) &&
		FMath::IsNearlyEqual(TintStrength, TargetTintStrength))
	{
		bIsTintTransitioning = false;

		SetActorTickEnabled(false);

		return;
	}

	// Start the linear transition
	bIsTintTransitioning = true;

	// Enable Tick while the tint changes
	SetActorTickEnabled(true);
}

// *******************************************************************************
//                             UPDATE TINT TRANSITION
// *******************************************************************************
void ACPP_TSWGameMode::UpdateTintTransition(float DeltaTime)
{
	// Make sure a transition is active
	if (!bIsTintTransitioning)
	{
		return;
	}

	// Increase elapsed time
	TintTransitionElapsed += DeltaTime;

	// Calculate the transition progress
	const float Alpha = FMath::Clamp(
		TintTransitionElapsed / TintTransitionDuration,
		0.0f,
		1.0f
	);

	// Interpolate the tint color linearly
	TintColor = FMath::Lerp(
		InitialTintColor,
		TargetTintColor,
		Alpha
	);

	// Interpolate the tint strength linearly
	TintStrength = FMath::Lerp(
		InitialTintStrength,
		TargetTintStrength,
		Alpha
	);

	// Update the material parameters
	UpdateTintMaterial();

	// Check whether the transition has finished
	if (Alpha >= 1.0f)
	{
		// Ensure the final values are exact
		TintColor = TargetTintColor;
		TintStrength = TargetTintStrength;

		UpdateTintMaterial();

		// Stop the transition
		bIsTintTransitioning = false;

		// Disable Tick until another transition starts
		SetActorTickEnabled(false);
	}
}

// *******************************************************************************
//                             UPDATE TINT MATERIAL
// *******************************************************************************
void ACPP_TSWGameMode::UpdateTintMaterial()
{
	// Make sure the dynamic material exists
	if (!DarknessMaterialDynamic)
	{
		return;
	}

	// Update the tint color
	DarknessMaterialDynamic->SetVectorParameterValue(
		TEXT("TintColor"),
		TintColor
	);

	// Update the tint strength
	DarknessMaterialDynamic->SetScalarParameterValue(
		TEXT("TintStrength"),
		TintStrength
	);
}

// *******************************************************************************
//                             RESET TINT
// *******************************************************************************
void ACPP_TSWGameMode::ResetTint(float TransitionDuration)
{
	// Restore the original scene colors using a linear transition
	SetTint(
		FLinearColor::White,
		0.0f,
		TransitionDuration
	);
}

// *******************************************************************************
//                             APPLY POST PROCESS TO CAMERA
// *******************************************************************************
void ACPP_TSWGameMode::ApplyPostProcessToCamera(UCameraComponent* Camera)
{
	// Make sure the camera and material exist
	if (!IsValid(Camera) || !IsValid(DarknessMaterialDynamic))
	{
		return;
	}

	// Apply the same dynamic material to the specified camera
	// ?: This keeps darkness, tint color and tint strength synchronized
	Camera->AddOrUpdateBlendable(
		DarknessMaterialDynamic,
		1.0f
	);
}
