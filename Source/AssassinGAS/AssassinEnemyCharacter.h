// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssassinCharacterBase.h"
#include "AssassinEnemyCharacter.generated.h"

class UStaticMeshComponent;

/**
 * 
 */
UCLASS()
class ASSASSINGAS_API AAssassinEnemyCharacter : public AAssassinCharacterBase
{
	GENERATED_BODY()
	
public:
	AAssassinEnemyCharacter();
	
	const TArray<TObjectPtr<AActor>>& GetPatrolPoints() const { return PatrolPoints; }
protected:
	UPROPERTY(EditInstanceOnly, Category = "AI|Patrol")
	TArray<TObjectPtr<AActor>> PatrolPoints;
};
