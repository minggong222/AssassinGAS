
#include "AssassinAttackTask.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "AssassinCharacterBase.h"
#include "AssassinGameplayTags.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameplayTagContainer.h"

UAssassinAttackTask::UAssassinAttackTask()
{
    NodeName = TEXT("Activate Attack Ability");
    bCreateNodeInstance = true;
    bNotifyTick = true;
}

EBTNodeResult::Type UAssassinAttackTask::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    (void)NodeMemory;

    AAIController* AIController = OwnerComp.GetAIOwner();
    AAssassinCharacterBase* Enemy = AIController ? Cast<AAssassinCharacterBase>(AIController->GetPawn()) : nullptr;
    UAbilitySystemComponent* ASC = Enemy ? Enemy->GetAbilitySystemComponent() : nullptr;

    if (!ASC)
    {
        UE_LOG(LogTemp, Error, TEXT("[EnemyAI][Combat] ASC Not Found"));
        return EBTNodeResult::Failed;
    }

    ActiveASC = ASC;
    ElapsedTime = 0.0f;
    bObservedAttacking = ASC->HasMatchingGameplayTag(AssassinGameplayTags::State_Attacking);

    if (!bObservedAttacking)
    {
        FGameplayTagContainer AttackTags;
        AttackTags.AddTag(AssassinGameplayTags::Ability_Attack);

        if (!ASC->TryActivateAbilitiesByTag(AttackTags, true))
        {
            UE_LOG(LogTemp, Warning, TEXT("[EnemyAI][Combat] Attack Activation Failed | Enemy=%s"), *Enemy->GetName());
            ActiveASC.Reset();
            return EBTNodeResult::Failed;
        }

        bObservedAttacking = ASC->HasMatchingGameplayTag(AssassinGameplayTags::State_Attacking);
    }

    UE_LOG(LogTemp, Display, TEXT("[EnemyAI][Combat] Attack Started | Enemy=%s | Attacking=%d"), *Enemy->GetName(), bObservedAttacking);

    return EBTNodeResult::InProgress;
}

void UAssassinAttackTask::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    (void)NodeMemory;

    UAbilitySystemComponent* ASC = ActiveASC.Get();

    if (!ASC)
    {
        ActiveASC.Reset();
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    ElapsedTime += DeltaSeconds;

    const bool bIsAttacking = ASC->HasMatchingGameplayTag(AssassinGameplayTags::State_Attacking);

    if (bIsAttacking)
    {
        bObservedAttacking = true;
        return;
    }

    if (bObservedAttacking)
    {
        UE_LOG(LogTemp, Display, TEXT("[EnemyAI][Combat] Attack Completed | Enemy=%s"), *GetNameSafe(ASC->GetOwner()));

        ActiveASC.Reset();
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    if (ElapsedTime >= 0.5f)
    {
        UE_LOG(LogTemp, Warning, TEXT("[EnemyAI][Combat] Attack State Not Applied | Enemy=%s"), *GetNameSafe(ASC->GetOwner()));

        ActiveASC.Reset();
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
    }
}

EBTNodeResult::Type UAssassinAttackTask::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    (void)OwnerComp;
    (void)NodeMemory;

    UAbilitySystemComponent* ASC = ActiveASC.Get();

    if (ASC && ASC->HasMatchingGameplayTag(AssassinGameplayTags::State_Attacking))
    {
        FGameplayTagContainer AttackTags;
        AttackTags.AddTag(AssassinGameplayTags::Ability_Attack);
        ASC->CancelAbilities(&AttackTags);

        UE_LOG(LogTemp, Warning, TEXT("[EnemyAI][Combat] Attack Aborted | Enemy=%s"), *GetNameSafe(ASC->GetOwner()));
    }

    ActiveASC.Reset();
    bObservedAttacking = false;
    ElapsedTime = 0.0f;

    return EBTNodeResult::Aborted;
}
