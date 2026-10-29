// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAssets/AI/Enemy/CPP_AttackPlayer_BTTask.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MyAssets/Characters/Enemy/CPP_EnemyAiController.h"
#include "MyAssets/Characters/Enemy/CPP_Enemy_Plush.h"
#include "MyAssets/Characters/Enemy/CPP_Enemy_Master.h"
#include "MyAssets/Characters/Enemy/CPP_EnemyActionsInterface.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyAssets/Characters/CPP_PlayerChar.h"
#include "Kismet/GameplayStatics.h"

UCPP_AttackPlayer_BTTask::UCPP_AttackPlayer_BTTask(FObjectInitializer const& ObjectInitializer) : UBTTask_BlackboardBase(ObjectInitializer)
{
	NodeName = TEXT("Attack Player");
	bNotifyTick = true;
}

//When executed it will find the patrol path on the next index so that the ai can move towrds it
EBTNodeResult::Type UCPP_AttackPlayer_BTTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (auto* const enemyController = Cast<ACPP_EnemyAiController>(OwnerComp.GetAIOwner()))
	{
		if (auto* const enemy = Cast<ACPP_Enemy_Master>(enemyController->GetPawn()))
		{
			if (enemy->GetClass()->ImplementsInterface(UCPP_EnemyActionsInterface::StaticClass())) {
				ICPP_EnemyActionsInterface::Execute_AttackPlayer(enemy);

				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return EBTNodeResult::Succeeded;

			}
			
		}
	}

	return EBTNodeResult::Failed;

}