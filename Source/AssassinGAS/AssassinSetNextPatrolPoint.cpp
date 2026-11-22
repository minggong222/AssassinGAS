// Fill out your copyright notice in the Description page of Project Settings.


#include "AssassinSetNextPatrolPoint.h"
#include "AssassinEnemyCharacter.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "NavigationSystem.h"
#include "NavigationData.h"

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

	const FVector GoalLocation = PatrolPoint->GetActorLocation();
	Blackboard->SetValueAsVector(PatrolLocationKey.SelectedKeyName, GoalLocation);
	Blackboard->SetValueAsInt(PatrolIndexKey.SelectedKeyName, (PointIndex + 1) % PointCount);

	const FVector AgentLocation = AIController->GetNavAgentLocation();
	const FVector BlackboardLocation = Blackboard->GetValueAsVector(PatrolLocationKey.SelectedKeyName);
	const FNavAgentProperties& AgentProperties = Enemy->GetNavAgentPropertiesRef();
	UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(Enemy->GetWorld());
	const ANavigationData* NavData = NavigationSystem ? NavigationSystem->GetNavDataForProps(AgentProperties) : nullptr;

	FNavLocation ProjectedStart;
	FNavLocation ProjectedGoal;
	const FVector ProjectionExtent(200.0f, 200.0f, 300.0f);

	const bool bStartProjected = NavigationSystem && NavData && NavigationSystem->ProjectPointToNavigation(AgentLocation, ProjectedStart, ProjectionExtent, NavData, nullptr);
	const bool bGoalProjected = NavigationSystem && NavData && NavigationSystem->ProjectPointToNavigation(GoalLocation, ProjectedGoal, ProjectionExtent, NavData, nullptr);

	bool bPathFound = false;

	if (bStartProjected && bGoalProjected)
	{
	    FPathFindingQuery Query(Enemy, *NavData, ProjectedStart.Location, ProjectedGoal.Location);
	    const FPathFindingResult PathResult = NavigationSystem->FindPathSync(Query, EPathFindingMode::Regular);
	    bPathFound = PathResult.IsSuccessful();
	}

	UE_LOG(LogTemp, Warning, TEXT("[EnemyAI][NavCheck] Key=%s | BB=%s | Target=%s | AgentRadius=%.1f | AgentHeight=%.1f | NavData=%s | StartProjected=%d | GoalProjected=%d | PathFound=%d"),
	    *PatrolLocationKey.SelectedKeyName.ToString(),
	    *BlackboardLocation.ToString(),
	    *GoalLocation.ToString(),
	    AgentProperties.AgentRadius,
	    AgentProperties.AgentHeight,
	    *GetNameSafe(NavData),
	    bStartProjected,
	    bGoalProjected,
	    bPathFound);

	UE_LOG(LogTemp, Display, TEXT("[EnemyAI][Patrol] Selected | Point=%s | Index=%d/%d"), *PatrolPoint->GetName(), PointIndex, PointCount);

		return EBTNodeResult::Succeeded;
	}

	UE_LOG(LogTemp, Warning, TEXT("[EnemyAI][Patrol] All Patrol Points are Invalid | Enemy=%s"), *Enemy->GetName());
	return EBTNodeResult::Failed;
}