// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAssets/AI/Enemy/CPP_ClimbWall_BTTask.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MyAssets/Characters/Enemy/CPP_EnemyAiController.h"
#include "MyAssets/Characters/Enemy/CPP_Enemy_Plush.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyAssets/Characters/CPP_PlayerChar.h"
#include "Kismet/GameplayStatics.h"

UCPP_ClimbWall_BTTask::UCPP_ClimbWall_BTTask(FObjectInitializer const& ObjectInitializer) : UBTTask_BlackboardBase(ObjectInitializer)
{
	NodeName = TEXT("Enemy Climb");
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
		
			if (FollowingPlayer) {
				if (auto* enemy = Cast<ACPP_Enemy_Plush>(controller->GetPawn()))
				{

					if (auto* player = Cast<ACPP_PlayerChar>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))) {


						float DirectionZ = player->GetActorLocation().Z - enemy->GetActorLocation().Z;

						int32 newClimbingDirection;
						newClimbingDirection = DirectionZ > 0.f ? 1 : -1;


						enemy->AddMovementInput(FVector(0, 0, 1), newClimbingDirection);

						float DirectionX = player->GetActorLocation().X - enemy->GetActorLocation().X;
						int32 newFacingDirection;
						newFacingDirection = DirectionX > 0.f ? 1 : -1;
						enemy->SetFacingDirection(newFacingDirection);
						if (!FMath::IsNearlyZero(DirectionX, 1.f)) {
							enemy->AddMovementInput(FVector(1, 0, 0), newFacingDirection);
						}

						/*if (enemy->GetClass()->ImplementsInterface(UCPP_EnemyActionsInterface::StaticClass())) {
							ICPP_EnemyActionsInterface::Execute_TurnEnemy(enemy, true);
						}*/

						//enemy->AddMovementInput(FVector(1, 0, 0), newFacingDirection);
					}
				}
				
			}
			else {
				//Enemy
				if (auto* enemy = Cast<ACPP_Enemy_Plush>(controller->GetPawn()))
				{
					enemy->AddMovementInput(FVector(0, 0, 1), 1);

					if (!enemy->WasRecentlyRendered() && enemy->GetEnemyIsStuckClimbing())
					{
						GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, "RENDER UPON ME");
						if (enemy->GetClass()->ImplementsInterface(UCPP_EnemyActionsInterface::StaticClass())) {
							ICPP_EnemyActionsInterface::Execute_EnemyIsLost(enemy);
						}
						enemy->SetEnemyIsStuckClimbing(false);

					}
				}
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
			/*if (enemy->GetClass()->ImplementsInterface(UCPP_EnemyActionsInterface::StaticClass())) {
				ICPP_EnemyActionsInterface::Execute_TurnEnemy(enemy, false);
			}*/

		}

	}

	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, "Successfully aborted");

	return EBTNodeResult::Aborted;
}