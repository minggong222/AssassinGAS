
#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AssassinAttackTask.generated.h"

class UAbilitySystemComponent;
class UBehaviorTreeComponent;

UCLASS()
class ASSASSINGAS_API UAssassinAttackTask : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UAssassinAttackTask();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
	TWeakObjectPtr<UAbilitySystemComponent> ActiveASC;
	bool bObservedAttacking = false;
	float ElapsedTime = 0.0f;
};
