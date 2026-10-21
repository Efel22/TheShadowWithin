// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAssets/Characters/Enemy/CPP_EnemyAiController.h"
#include "MyAssets/Characters/Enemy/CPP_Enemy_Master.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "MyAssets/Characters/CPP_PlayerChar.h"


ACPP_EnemyAiController::ACPP_EnemyAiController(FObjectInitializer const& ObjectInitializer) {
	SetupPerceptionSystem();
}

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

void ACPP_EnemyAiController::SetupPerceptionSystem() {
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("Sight Config"));

	if (SightConfig) {
		SetPerceptionComponent(*CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("Perception Component")));
		SightConfig->SightRadius = 500.f;
		SightConfig->LoseSightRadius = SightConfig->SightRadius = 25.f;
		SightConfig->PeripheralVisionAngleDegrees = 90.f;
		SightConfig->SetMaxAge(5.f);
		SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.f;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

		GetPerceptionComponent()->SetDominantSense(*SightConfig->GetSenseImplementation());
		GetPerceptionComponent()->OnTargetPerceptionUpdated.AddDynamic(this, &ACPP_EnemyAiController::OnTargetDetected);
		GetPerceptionComponent()->ConfigureSense(*SightConfig);
	}
}

//When the AI sees the player set the can see playerbool to true to execute diferent task
void ACPP_EnemyAiController::OnTargetDetected(AActor* Actor, FAIStimulus const Stimulus) {
	if (auto* const ch = Cast<ACPP_PlayerChar>(Actor)) {
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, "I FOUND YOU!!!");
		GetBlackboardComponent()->SetValueAsBool("CanSeePlayer", Stimulus.WasSuccessfullySensed());
	}

	/*if (auto* const ch = Cast<ASpaceship_CPP>(Actor)) {
		GetBlackboardComponent()->SetValueAsBool("CanSeePlayer", Stimulus.WasSuccessfullySensed());
	}*/
}