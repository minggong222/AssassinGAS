// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "AssassinSetNextPatrolPoint.generated.h"

/**
 * 
 */

class UBehaviorTree;
class UBehaviorTreeComponent;

UCLASS()
class ASSASSINGAS_API UAssassinSetNextPatrolPoint : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UAssassinSetNextPatrolPoint();
	
	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;
protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
private:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PatrolLocationKey;
	
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector PatrolIndexKey;
};
