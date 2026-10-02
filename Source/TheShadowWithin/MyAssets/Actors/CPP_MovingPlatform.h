// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InterpToMovementComponent.h"
#include "Components/BoxComponent.h" 
#include "CPP_MovingPlatform.generated.h"

class UPaperSpriteComponent; // Required for sprite handling

UCLASS()
class THESHADOWWITHIN_API ACPP_MovingPlatform : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACPP_MovingPlatform();

protected:

	// Stores the platform's sprite
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components|Visual")
	UPaperSpriteComponent* PlatformSpriteComp;

	// Stores the interp. movement component
	// WHY?: Allows for platform editing IN the editor itself 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components|Movement")
	UInterpToMovementComponent* InterpMovComp;

	// Stores the box component (this is the "TRUE" collision box of the sprite
	// WHY?: This allows us to edit the platform sprite however we like without messing with its
	//       collision!
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components|Movement")
	UBoxComponent* CollisionBox; 

};
