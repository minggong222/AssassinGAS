// Fill out your copyright notice in the Description page of Project Settings.


#include "AssassinAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "AssassinGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AssassinCharacterBase.h"

UAssassinAttributeSet::UAssassinAttributeSet()
	: Health(100.0f),
	  MaxHealth(100.0f),
	  Stamina(100.0f),
	  MaxStamina(100.0f),
	  AttackPower(20.0f),
	  Defense(2.0f),
	  CriticalChance(10.0f),
	  CriticalDamageMultiplier(2.0f)
{
}

void UAssassinAttributeSet::ClampAttributeValue(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxHealth());
	}
	else if (Attribute == GetMaxHealthAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, GetMaxStamina());
	}
	else if (Attribute == GetMaxStaminaAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}
	else if (Attribute == GetAttackPowerAttribute() || Attribute == GetDefenseAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.0f);
	}else if (Attribute == GetCriticalChanceAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, 100.0f);
	}
	else if (Attribute == GetCriticalDamageMultiplierAttribute())
	{
		NewValue = FMath::Max(NewValue, 1.0f);
	}
}

void UAssassinAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	ClampAttributeValue(Attribute, NewValue);
}

void UAssassinAttributeSet::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	ClampAttributeValue(Attribute, NewValue);
}


void UAssassinAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	const FGameplayAttribute& Attribute = Data.EvaluatedData.Attribute;

	if (Attribute == GetHealthAttribute() || Attribute == GetMaxHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
	}
	else if (Attribute == GetStaminaAttribute() || Attribute == GetMaxStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.0f, GetMaxStamina()));
	}

	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();

	if (GetHealth() <= 0.0f && ASC && !ASC->HasMatchingGameplayTag(AssassinGameplayTags::State_Dead))
	{
		ASC->AddLooseGameplayTag(AssassinGameplayTags::State_Dead, 1, EGameplayTagReplicationState::None);
		ASC->CancelAllAbilities(nullptr);

		if (AAssassinCharacterBase* Character = Cast<AAssassinCharacterBase>(ASC->GetAvatarActor()))
		{
			Character->HandleDeath();
		}

		UE_LOG(LogTemp, Warning, TEXT("[GAS][Death] State.Dead applied. Health=%.1f"), GetHealth());
	}
	else if (Attribute == GetHealthAttribute() && Data.EvaluatedData.Magnitude < 0.0f && ASC && GetHealth() > 0.0f && !ASC->HasMatchingGameplayTag(AssassinGameplayTags::State_Dead))
	{
		FGameplayTagContainer AbilityTags;
		AbilityTags.AddTag(AssassinGameplayTags::Ability_HitReact);

		const bool bActivated = ASC->TryActivateAbilitiesByTag(AbilityTags, true);

		UE_LOG(LogTemp, Display, TEXT("[GAS][HitReact] Activated=%d | Target=%s | Health=%.1f"), bActivated, *GetNameSafe(ASC->GetAvatarActor()), GetHealth());
	}
}
