// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAssets/AI/Enemy/CPP_SummonEnemy_BTTask.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MyAssets/Characters/Enemy/CPP_EnemyAiController.h"
#include "MyAssets/Characters/Enemy/CPP_Enemy_Master.h"
#include "MyAssets/Characters/Enemy/CPP_Enemy_Plush.h"
#include "MyAssets/Characters/Enemy/CPP_Enemy_Boss.h"

UCPP_SummonEnemy_BTTask::UCPP_SummonEnemy_BTTask(FObjectInitializer const& ObjectInitializer) : UBTTask_BlackboardBase(ObjectInitializer)
{
	NodeName = TEXT("Summon Enemy");
}

//When executed it will find the patrol path on the next index so that the ai can move towrds it
EBTNodeResult::Type UCPP_SummonEnemy_BTTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	//Enemy controller
	if (auto* const enemyController = Cast<ACPP_EnemyAiController>(OwnerComp.GetAIOwner()))
	{
		//Blackboard components
		if (auto* const bc = OwnerComp.GetBlackboardComponent())
		{
			//integer
			auto const index = bc->GetValueAsInt(GetSelectedBlackboardKey());
			//Enemy
			if (auto* enemy = Cast<ACPP_Enemy_Boss>(enemyController->GetPawn()))
			{
				// Make sure the index is within the array bounds
				if (index < 0 || index >= enemy->GetSummonedEnemyNum())
				{
					
					UE_LOG(LogTemp, Warning,
						TEXT("MoveToPoint: Invalid patrol index %d (Array size: %d)"),
						index,
						enemy->GetSummonedEnemyNum());
					return EBTNodeResult::Failed;
				}
				auto* summonedEnemy = enemy->GetSummonedEnemy(index);

				summonedEnemy->SetActorLocation(enemy->GetSpawnLocation());

				if (summonedEnemy->GetClass()->ImplementsInterface(UCPP_EnemyActionsInterface::StaticClass())) {
					ICPP_EnemyActionsInterface::Execute_SummonEnemy(summonedEnemy);
					//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, "Summon Enemy");
				}

				if (enemy->GetClass()->ImplementsInterface(UCPP_EnemyActionsInterface::StaticClass())) {
					ICPP_EnemyActionsInterface::Execute_HasSpawnLimitReached(enemy, 1);
					//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Red, "Summon Enemy");
				}

				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return EBTNodeResult::Succeeded;
			}
		}
	}
	return EBTNodeResult::Failed;

}
