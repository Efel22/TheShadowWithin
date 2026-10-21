// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAssets/AI/Enemy/CPP_ClimbWall_BTTask.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MyAssets/Characters/Enemy/CPP_EnemyAiController.h"
#include "MyAssets/Characters/Enemy/CPP_Enemy_Plush.h"
#include "GameFramework/CharacterMovementComponent.h"

UCPP_ClimbWall_BTTask::UCPP_ClimbWall_BTTask(FObjectInitializer const& ObjectInitializer) : UBTTask_BlackboardBase(ObjectInitializer)
{
	NodeName = TEXT("Move Enemy");
	bNotifyTick = true;
}

//When executed it will find the patrol path on the next index so that the ai can move towrds it
EBTNodeResult::Type UCPP_ClimbWall_BTTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto* const controller = Cast<ACPP_EnemyAiController>(OwnerComp.GetAIOwner()))
	{
		//Blackboard components
		
			//Enemy
			if (auto* enemy = Cast<ACPP_Enemy_Plush>(controller->GetPawn()))
			{
				enemy->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
				
				
			}
		
	}

	return EBTNodeResult::InProgress;

}

void UCPP_ClimbWall_BTTask::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) {
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	//Enemy controller
	if (auto* const controller = Cast<ACPP_EnemyAiController>(OwnerComp.GetAIOwner()))
	{
		
			
			//Enemy
			if (auto* enemy = Cast<ACPP_Enemy_Plush>(controller->GetPawn()))
			{
				enemy->AddMovementInput(FVector(0, 0, 1), 1);
			}
		
	}

}

EBTNodeResult::Type UCPP_ClimbWall_BTTask::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {
	if (auto* const controller = Cast<ACPP_EnemyAiController>(OwnerComp.GetAIOwner()))
	{
		//Blackboard components

			//Enemy
		if (auto* enemy = Cast<ACPP_Enemy_Plush>(controller->GetPawn()))
		{
			enemy->GetCharacterMovement()->SetMovementMode(MOVE_Walking);


		}

	}

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, "Successfully aborted");

	return EBTNodeResult::Aborted;
}