// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAssets/AI/Enemy/CPP_MoveEnemy_BTTask.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MyAssets/Characters/Enemy/CPP_EnemyAiController.h"
#include "MyAssets/Characters/Enemy/CPP_Enemy_Master.h"

UCPP_MoveEnemy_BTTask::UCPP_MoveEnemy_BTTask(FObjectInitializer const& ObjectInitializer) : UBTTask_BlackboardBase(ObjectInitializer)
{
	NodeName = TEXT("Move Enemy");
	bNotifyTick = true;
}

//When executed it will find the patrol path on the next index so that the ai can move towrds it
EBTNodeResult::Type UCPP_MoveEnemy_BTTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::InProgress;

}

void UCPP_MoveEnemy_BTTask::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) {
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

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

				float DirectionX = GlobalPoint.X - enemy->GetActorLocation().X;

				float distance = FVector::Dist(GlobalPoint, enemy->GetActorLocation());

				// Update the enemy's facing direction
				// **NOTE: Prevents changing direction if the player is directly above/below
				
				if (distance > offset) {


					int32 newFacingDirection;
					newFacingDirection = DirectionX > 0.f ? 1 : -1;
					enemy->AddMovementInput(FVector(1, 0, 0), newFacingDirection);
					GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, "Moving");
				}
				else {
					FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
				}
					
				









			}
		}
	}
	
}