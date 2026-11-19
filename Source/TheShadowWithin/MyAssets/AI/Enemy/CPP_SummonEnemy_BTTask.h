// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "CPP_SummonEnemy_BTTask.generated.h"

/**
 * 
 */
UCLASS()
class THESHADOWWITHIN_API UCPP_SummonEnemy_BTTask : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	explicit UCPP_SummonEnemy_BTTask(FObjectInitializer const& ObjectInitializer);
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;


};
