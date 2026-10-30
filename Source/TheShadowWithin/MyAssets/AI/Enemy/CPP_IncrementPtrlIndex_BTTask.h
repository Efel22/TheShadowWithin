// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlackboardBase.h"
#include "CPP_IncrementPtrlIndex_BTTask.generated.h"

/**
 * 
 */
UCLASS()
class THESHADOWWITHIN_API UCPP_IncrementPtrlIndex_BTTask : public UBTTask_BlackboardBase
{
	GENERATED_BODY()
	
public:
	explicit UCPP_IncrementPtrlIndex_BTTask(FObjectInitializer const& ObjectInitializer);
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	enum class EDirectionType { Forward, Reverse };

	EDirectionType Direction = EDirectionType::Forward;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI", meta = (AllowPrivateAccess = "true"))
	bool bDirectional = false;

};
