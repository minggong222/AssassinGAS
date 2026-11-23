// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "AssassinEnemyAIController.generated.h"

/**
 * 
 */
class UBehaviorTree;
class UAISenseConfig_Sight;
class UAISenseConfig_Hearing;
class AAssassinGASCharacter;

UCLASS()
class ASSASSINGAS_API AAssassinEnemyAIController : public AAIController
{
	GENERATED_BODY()
public:
	AAssassinEnemyAIController();
	
	void RefreshPlayerStealthState(AAssassinGASCharacter* Player);
protected:
	virtual void OnPossess(APawn* InPawn) override;
	
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY(VisibleAnywhere, Category = "AI|Perception")
	TObjectPtr<UAISenseConfig_Hearing> HearingConfig;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;
};
