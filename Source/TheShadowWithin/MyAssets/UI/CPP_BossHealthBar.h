// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CPP_BossHealthBar.generated.h"

/**
 * 
 */
UCLASS()
class THESHADOWWITHIN_API UCPP_BossHealthBar : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void SetBossHealthBar(int currentHealth);
	

	UFUNCTION(BlueprintImplementableEvent)
	void SetBossHealthBarMax(int maxHealth);

};
