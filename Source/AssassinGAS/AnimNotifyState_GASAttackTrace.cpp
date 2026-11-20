#include "AnimNotifyState_GASAttackTrace.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AssassinGameplayTags.h"
#include "Components/SkeletalMeshComponent.h"
#include "Abilities/GameplayAbilityTypes.h"

void UAnimNotifyState_GASAttackTrace::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (!MeshComp)
	{
		return;
	}
	
	AActor* Owner = MeshComp->GetOwner();
	const IAbilitySystemInterface* AbilitySystemInterface = Owner ? Cast<IAbilitySystemInterface>(Owner) : nullptr;
	UAbilitySystemComponent* ASC = AbilitySystemInterface ? AbilitySystemInterface->GetAbilitySystemComponent() : nullptr;
	
	if (!ASC)
	{
		return;
	}
	
	FGameplayEventData Payload;
	Payload.EventTag = AssassinGameplayTags::Event_Attack_Trace_Begin;
	Payload.Instigator = Owner;
	Payload.OptionalObject = this;
	
	ASC->HandleGameplayEvent(AssassinGameplayTags::Event_Attack_Trace_Begin, &Payload);
}

void UAnimNotifyState_GASAttackTrace::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	if (!MeshComp)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();

	if (!Owner)
	{
		return;
	}

	const IAbilitySystemInterface* AbilitySystemInterface = Cast<IAbilitySystemInterface>(Owner);
	UAbilitySystemComponent* ASC = AbilitySystemInterface ? AbilitySystemInterface->GetAbilitySystemComponent() : nullptr;

	if (!ASC)
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = AssassinGameplayTags::Event_Attack_Hit;
	Payload.Instigator = Owner;
	Payload.OptionalObject = this;
	
	ASC->HandleGameplayEvent(AssassinGameplayTags::Event_Attack_Hit, &Payload);
}

void UAnimNotifyState_GASAttackTrace::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
}