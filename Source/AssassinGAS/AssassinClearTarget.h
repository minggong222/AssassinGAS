
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "AssassinClearTarget.generated.h"

class UBehaviorTree;
class UBehaviorTreeComponent;

UCLASS()
class ASSASSINGAS_API UAssassinClearTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAssassinClearTarget();

	virtual void InitializeFromAsset(UBehaviorTree& Asset) override;

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;
};
