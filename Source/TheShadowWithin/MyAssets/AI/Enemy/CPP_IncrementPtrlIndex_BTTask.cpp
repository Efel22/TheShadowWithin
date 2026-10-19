// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAssets/AI/Enemy/CPP_IncrementPtrlIndex_BTTask.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MyAssets/Characters/Enemy/CPP_EnemyAiController.h"
#include "MyAssets/Characters/Enemy/CPP_Enemy_Master.h"

UCPP_IncrementPtrlIndex_BTTask::UCPP_IncrementPtrlIndex_BTTask(FObjectInitializer const& ObjectInitializer) : UBTTask_BlackboardBase(ObjectInitializer)
{
	NodeName = "Increment Path Index";
}

//Simply increases the the index so that the ai can find the next patrol point. when it reaches the max it will go on reverse now
EBTNodeResult::Type UCPP_IncrementPtrlIndex_BTTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto* const enemyController = Cast<ACPP_EnemyAiController>(OwnerComp.GetAIOwner()))
	{
		if (auto* const enemy = Cast<ACPP_Enemy_Master>(enemyController->GetPawn()))
		{

			if (auto* const bc = OwnerComp.GetBlackboardComponent())
			{
				auto const noOfPoints = enemy->GetPatrolPoint()->Num();
				auto const minIndex = 0;
				auto const maxIndex = noOfPoints - 1;
				auto index = bc->GetValueAsInt(GetSelectedBlackboardKey());
				if (bDirectional) {
					if (index >= maxIndex && Direction == EDirectionType::Forward) {
						Direction = EDirectionType::Reverse;
					}
					else if (index == minIndex && Direction == EDirectionType::Reverse) {
						Direction = EDirectionType::Forward;
					}
				}
				bc->SetValueAsInt(GetSelectedBlackboardKey(), (Direction == EDirectionType::Forward ? ++index : --index) % noOfPoints);

				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return EBTNodeResult::Succeeded;
			}
		}
	}

	return EBTNodeResult::Failed;
}
