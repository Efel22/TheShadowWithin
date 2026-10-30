// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAssets/AI/Enemy/CPP_MoveToPoint_BTTask.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MyAssets/Characters/Enemy/CPP_EnemyAiController.h"
#include "MyAssets/Characters/Enemy/CPP_Enemy_Master.h"

UCPP_MoveToPoint_BTTask::UCPP_MoveToPoint_BTTask(FObjectInitializer const& ObjectInitializer) : UBTTask_BlackboardBase(ObjectInitializer)
{
	NodeName = TEXT("Find Patrol Point");
}

//When executed it will find the patrol path on the next index so that the ai can move towrds it
EBTNodeResult::Type UCPP_MoveToPoint_BTTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
			if (auto* enemy = Cast<ACPP_Enemy_Master>(enemyController->GetPawn()))
			{
				//Patrol point
				auto const Point = enemy->GetPatrolPoint()->GetOnePatrolPoint(index);

				auto const GlobalPoint = enemy->GetPatrolPoint()->GetActorTransform().TransformPosition(Point);
				bc->SetValueAsVector(PatrolPathVectorKey.SelectedKeyName, GlobalPoint);

				float DirectionX = GlobalPoint.X - enemy->GetActorLocation().X;

				// Update the enemy's facing direction
				// **NOTE: Prevents changing direction if the player is directly above/below
				if (!FMath::IsNearlyZero(DirectionX, 1.f))
				{
					int32 newFacingDirection;
					newFacingDirection = DirectionX > 0.f ? 1 : -1;
					enemy->SetFacingDirection(newFacingDirection);
				}



				enemy->UpdateSpriteRotation();

				FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				return EBTNodeResult::Succeeded;
			}
		}
	}
	return EBTNodeResult::Failed;
	
}