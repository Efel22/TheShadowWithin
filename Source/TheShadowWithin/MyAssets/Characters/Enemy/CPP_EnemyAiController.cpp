// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAssets/Characters/Enemy/CPP_EnemyAiController.h"
#include "MyAssets/Characters/Enemy/CPP_Enemy_Master.h"

void ACPP_EnemyAiController::OnPossess(APawn* InPawn) {
	Super::OnPossess(InPawn);
	if (ACPP_Enemy_Master* const enemy = Cast<ACPP_Enemy_Master>(InPawn)) {
		if (UBehaviorTree* const tree = enemy->GetBehaviorTree()) {
			UBlackboardComponent* b;
			UseBlackboard(tree->BlackboardAsset, b);
			Blackboard = b;
			RunBehaviorTree(tree);
		}
	}
}