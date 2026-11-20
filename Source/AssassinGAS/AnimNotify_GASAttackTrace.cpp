
#include "AnimNotify_GASAttackTrace.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AssassinGameplayTags.h"
#include "Components/SkeletalMeshComponent.h"
#include "Abilities/GameplayAbilityTypes.h"
void UAnimNotify_GASAttackTrace::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
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
		UE_LOG(LogTemp, Warning, TEXT("[AnimNotify][AttackTrace] ASC Not Found | Owner=%s"), *Owner->GetName());
		return;
	}

	FGameplayEventData Payload;
	Payload.EventTag = AssassinGameplayTags::Event_Attack_Hit;
	Payload.Instigator = Owner;

	ASC->HandleGameplayEvent(AssassinGameplayTags::Event_Attack_Hit, &Payload);

	UE_LOG(LogTemp, Display, TEXT("[AnimNotify][AttackTrace] Event Sent | Owner=%s | Event=Event.Attack.Hit"), *Owner->GetName());
}

FString UAnimNotify_GASAttackTrace::GetNotifyName_Implementation() const
{
	return TEXT("GAS Attack Trace");
}
