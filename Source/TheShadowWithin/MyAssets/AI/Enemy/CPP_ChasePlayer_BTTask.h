// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "CPP_ChasePlayer_BTTask.generated.h"

/**
 * 
 */
UCLASS()
class THESHADOWWITHIN_API UCPP_ChasePlayer_BTTask : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	explicit UCPP_ChasePlayer_BTTask(FObjectInitializer const& ObjectInitializer);
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
