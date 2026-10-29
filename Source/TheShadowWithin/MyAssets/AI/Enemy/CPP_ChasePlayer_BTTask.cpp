// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAssets/AI/Enemy/CPP_ChasePlayer_BTTask.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MyAssets/Characters/Enemy/CPP_EnemyAiController.h"
#include "MyAssets/Characters/Enemy/CPP_Enemy_Plush.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MyAssets/Characters/CPP_PlayerChar.h"
#include "Kismet/GameplayStatics.h"

UCPP_ChasePlayer_BTTask::UCPP_ChasePlayer_BTTask(FObjectInitializer const& ObjectInitializer) : UBTTask_BlackboardBase(ObjectInitializer)
{
	NodeName = TEXT("Chase Player");
	bNotifyTick = true;
}

//When executed it will find the patrol path on the next index so that the ai can move towrds it
EBTNodeResult::Type UCPP_ChasePlayer_BTTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return EBTNodeResult::InProgress;

}

void UCPP_ChasePlayer_BTTask::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) {
	Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

	//Enemy controller
	if (auto* const controller = Cast<ACPP_EnemyAiController>(OwnerComp.GetAIOwner()))
	{


		//Enemy
		if (auto* enemy = Cast<ACPP_Enemy_Plush>(controller->GetPawn()))
		{
			if (auto* player = Cast<ACPP_PlayerChar>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0))) {


				float DirectionX = player->GetActorLocation().X - enemy->GetActorLocation().X;

				int32 newFacingDirection;
				newFacingDirection = DirectionX > 0.f ? 1 : -1;
				enemy->SetFacingDirection(newFacingDirection);
				enemy->AddMovementInput(FVector(1, 0, 0), newFacingDirection);

			}

			enemy->UpdateSpriteRotation();
		}

	}

}

EBTNodeResult::Type UCPP_ChasePlayer_BTTask::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) {
	if (auto* const controller = Cast<ACPP_EnemyAiController>(OwnerComp.GetAIOwner()))
	{
		//Blackboard components

			//Enemy
		if (auto* enemy = Cast<ACPP_Enemy_Plush>(controller->GetPawn()))
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "I lost you");



		}

	}

	

	return EBTNodeResult::Aborted;
}