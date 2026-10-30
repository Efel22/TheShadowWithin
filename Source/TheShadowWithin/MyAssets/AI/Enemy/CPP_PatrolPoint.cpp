// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAssets/AI/Enemy/CPP_PatrolPoint.h"

// Sets default values
ACPP_PatrolPoint::ACPP_PatrolPoint()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

//Gets the index for the patrolling ai
FVector ACPP_PatrolPoint::GetOnePatrolPoint(int const index) const {
	return PatrolPoints[index];
}

//~~~Returns the number of elements in the array
int ACPP_PatrolPoint::Num() const {
	return PatrolPoints.Num();
}

