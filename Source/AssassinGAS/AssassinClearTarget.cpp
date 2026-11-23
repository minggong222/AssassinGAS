
#include "AssassinClearTarget.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BlackboardData.h"
#include "GameFramework/Actor.h"
#include "AIController.h"

UAssassinClearTarget::UAssassinClearTarget()
{
	NodeName = TEXT("Clear Target Actor");

	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UAssassinClearTarget, TargetActorKey), AActor::StaticClass());
}

void UAssassinClearTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BlackboardAsset = GetBlackboardAsset())
	{
		TargetActorKey.ResolveSelectedKey(*BlackboardAsset);
	}
}

EBTNodeResult::Type UAssassinClearTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	(void)NodeMemory;

	UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();

	if (!Blackboard || !TargetActorKey.IsSet())
	{
		UE_LOG(LogTemp, Error, TEXT("[EnemyAI][Search] Invalid Blackboard or TargetActor Key"));
		return EBTNodeResult::Failed;
	}

	AActor* PreviousTarget = Cast<AActor>(Blackboard->GetValueAsObject(TargetActorKey.SelectedKeyName));

	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		AIController->ClearFocus(EAIFocusPriority::Gameplay);
	}

	Blackboard->ClearValue(TargetActorKey.SelectedKeyName);

	UE_LOG(LogTemp, Display, TEXT("[EnemyAI][Search] Completed | ClearedTarget=%s"), *GetNameSafe(PreviousTarget));

	return EBTNodeResult::Succeeded;
}
