// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAssets/AI/Enemy/CPP_IncrementSummonIndex_BTTask.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MyAssets/Characters/Enemy/CPP_EnemyAiController.h"
#include "MyAssets/Characters/Enemy/CPP_Enemy_Master.h"
#include "MyAssets/Characters/Enemy/CPP_Enemy_Boss.h"


UCPP_IncrementSummonIndex_BTTask::UCPP_IncrementSummonIndex_BTTask(FObjectInitializer const& ObjectInitializer) : UBTTask_BlackboardBase(ObjectInitializer)
{
	NodeName = "Increment Summoned Enemy Index";
}

//Simply increases the the index so that the ai can find the next patrol point. when it reaches the max it will go on reverse now
EBTNodeResult::Type UCPP_IncrementSummonIndex_BTTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto* const enemyController = Cast<ACPP_EnemyAiController>(OwnerComp.GetAIOwner()))
	{
		if (auto* const enemy = Cast<ACPP_Enemy_Boss>(enemyController->GetPawn()))
		{

			if (auto* const bc = OwnerComp.GetBlackboardComponent())
			{
				auto const noOfPoints = enemy->GetSummonedEnemyNum();
				auto const minIndex = 0;
				auto const maxIndex = noOfPoints - 1;
				auto index = bc->GetValueAsInt(GetSelectedBlackboardKey());
				
					if (index >= maxIndex) {
						index = minIndex;
						//bc->SetValueAsBool(CanSpawnEnemiesBool.SelectedKeyName, true);
						GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "nothing happened");
					}
					else {
						index++;
						GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, "Increment Spawned enemy index");
					}
				
				bc->SetValueAsInt(GetSelectedBlackboardKey(),  index % noOfPoints);

				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return EBTNodeResult::Succeeded;
			}
		}
	}

	return EBTNodeResult::Failed;
}
