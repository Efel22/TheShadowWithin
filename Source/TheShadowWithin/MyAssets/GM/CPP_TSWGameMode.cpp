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

	// Get a reference to the player's character
	ACPP_PlayerChar* PlayerChar = Cast<ACPP_PlayerChar>(
		UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)
	);

	// Make sure the player exists
	if (!PlayerChar)
	{
		return;
	}

	// Add the darkness post process material to the player's camera
	PlayerChar->GetCamera()->AddOrUpdateBlendable(
		DarknessMaterialDynamic,
		1.0f
	);

	//Start the darkness increment timer (its on a loop)
	StartDarknessTimer();

}

// *******************************************************************************
//                             START DARKNESS
// *******************************************************************************
void ACPP_TSWGameMode::StartDarknessTimer()
{
	if (DoesDarknessIncrements)
	{
		GetWorld()->GetTimerManager().SetTimer(
			DarknessTimer,
			[this]()
			{
				AddDarkness(DarknessIncrements);
			},
			SecondsBetweenIncrements,
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
	if (Darkness >= 1.0f)
	{
		
		ACPP_PlayerChar* PlayerRef = Cast<ACPP_PlayerChar>(
			UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)
		);

		PlayerRef->Die();
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
}