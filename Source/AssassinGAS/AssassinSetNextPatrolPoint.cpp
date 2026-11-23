// Fill out your copyright notice in the Description page of Project Settings.


#include "AssassinSetNextPatrolPoint.h"
#include "AssassinEnemyCharacter.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"

UAssassinSetNextPatrolPoint::UAssassinSetNextPatrolPoint()
{
	NodeName = TEXT("Select Next Patrol Point");
	
	PatrolLocationKey.AddVectorFilter(this, GET_MEMBER_NAME_CHECKED(UAssassinSetNextPatrolPoint, PatrolLocationKey));
	PatrolIndexKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(UAssassinSetNextPatrolPoint, PatrolIndexKey));
}

void UAssassinSetNextPatrolPoint::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BlackboardAsset = GetBlackboardAsset())
	{
		PatrolLocationKey.ResolveSelectedKey(*BlackboardAsset);
		PatrolIndexKey.ResolveSelectedKey(*BlackboardAsset);
	}
}

EBTNodeResult::Type UAssassinSetNextPatrolPoint::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	(void)NodeMemory;

	AAIController* AIController = OwnerComp.GetAIOwner();
	AAssassinEnemyCharacter* Enemy = AIController ? Cast<AAssassinEnemyCharacter>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!Enemy || !Blackboard || !PatrolLocationKey.IsSet() || !PatrolIndexKey.IsSet())
	{
		UE_LOG(LogTemp, Error, TEXT("[EnemyAI][Patrol] Invalid Reference or Blackboard Key"));
		return EBTNodeResult::Failed;
	}

	const TArray<TObjectPtr<AActor>>& PatrolPoints = Enemy->GetPatrolPoints();

	if (PatrolPoints.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("[EnemyAI][Patrol] No Patrol Points | Enemy=%s"), *Enemy->GetName());
		return EBTNodeResult::Failed;
	}

	const int32 PointCount = PatrolPoints.Num();
	const int32 CurrentIndex = Blackboard->GetValueAsInt(PatrolIndexKey.SelectedKeyName);
	const int32 StartIndex = ((CurrentIndex % PointCount) + PointCount) % PointCount;

	for (int32 Offset = 0; Offset < PointCount; ++Offset)
	{
		const int32 PointIndex = (StartIndex + Offset) % PointCount;
		AActor* PatrolPoint = PatrolPoints[PointIndex].Get();

		if (!IsValid(PatrolPoint))
		{
			continue;
		}

		Blackboard->SetValueAsVector(PatrolLocationKey.SelectedKeyName, PatrolPoint->GetActorLocation());
		Blackboard->SetValueAsInt(PatrolIndexKey.SelectedKeyName, (PointIndex + 1) % PointCount);

		UE_LOG(LogTemp, Display, TEXT("[EnemyAI][Patrol] Selected | Point=%s | Index=%d/%d"), *PatrolPoint->GetName(), PointIndex, PointCount);

		return EBTNodeResult::Succeeded;
	}

	UE_LOG(LogTemp, Warning, TEXT("[EnemyAI][Patrol] All Patrol Points are Invalid | Enemy=%s"), *Enemy->GetName());
	return EBTNodeResult::Failed;
}