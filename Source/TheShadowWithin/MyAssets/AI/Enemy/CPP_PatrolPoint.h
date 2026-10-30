// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CPP_PatrolPoint.generated.h"

UCLASS()
class THESHADOWWITHIN_API ACPP_PatrolPoint : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACPP_PatrolPoint();

protected:
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ai", meta = (MakeEditWidget = "true", AllowPrivateAccess = "true"))
	TArray<FVector> PatrolPoints;

public:	
	

	FVector GetOnePatrolPoint(int const index) const;
	int Num() const;

};
