// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AssassinEnemyAIController.generated.h"

/**
 * 
 */
class UBehaviorTree;

UCLASS()
class ASSASSINGAS_API AAssassinEnemyAIController : public AAIController
{
	GENERATED_BODY()
public:
	AAssassinEnemyAIController();
protected:
	virtual void OnPossess(APawn* InPawn) override;
	
	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;
};
