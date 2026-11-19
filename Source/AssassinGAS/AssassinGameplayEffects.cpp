// Fill out your copyright notice in the Description page of Project Settings.


#include "AssassinGameplayEffects.h"
#include "AssassinAttributeSet.h"
#include "ScalableFloat.h"

UAssassinDamageEffect::UAssassinDamageEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;
	
	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UAssassinAttributeSet::GetHealthAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(-20.0f));
	
	Modifiers.Add(Modifier);
}

UAssassinHealEffect::UAssassinHealEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UAssassinAttributeSet::GetHealthAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(30.0f));

	Modifiers.Add(Modifier);
}

UAssassinStaminaCostEffect::UAssassinStaminaCostEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UAssassinAttributeSet::GetStaminaAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(-15.0f));

	Modifiers.Add(Modifier);
}

UAssassinStaminaRestoreEffect::UAssassinStaminaRestoreEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = UAssassinAttributeSet::GetStaminaAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(FScalableFloat(20.0f));

	Modifiers.Add(Modifier);
}