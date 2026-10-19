// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "CPP_MoveToPoint_BTTask.generated.h"

/**
 * 
 */
UCLASS()
class THESHADOWWITHIN_API UCPP_MoveToPoint_BTTask : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	explicit UCPP_MoveToPoint_BTTask(FObjectInitializer const& ObjectInitializer);
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	FBlackboardKeySelector PatrolPathVectorKey;

};
