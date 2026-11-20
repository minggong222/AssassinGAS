// Fill out your copyright notice in the Description page of Project Settings.


#include "AssassinGameplayEffects.h"
#include "AssassinAttributeSet.h"
#include "AssassinGameplayTags.h"
#include "GameplayEffectAggregator.h"
#include "ScalableFloat.h"


UAssassinDamageExecution::UAssassinDamageExecution()
{
	AttackPowerCaptureDefinition = FGameplayEffectAttributeCaptureDefinition(UAssassinAttributeSet::GetAttackPowerAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	DefenseCaptureDefinition = FGameplayEffectAttributeCaptureDefinition(UAssassinAttributeSet::GetDefenseAttribute(), EGameplayEffectAttributeCaptureSource::Target, false);
	CriticalChanceCaptureDefinition  = FGameplayEffectAttributeCaptureDefinition(UAssassinAttributeSet::GetCriticalChanceAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	CriticalDamageMultiplierCaptureDefinition  = FGameplayEffectAttributeCaptureDefinition(UAssassinAttributeSet::GetCriticalDamageMultiplierAttribute(), EGameplayEffectAttributeCaptureSource::Source, true);
	
	RelevantAttributesToCapture.Add(AttackPowerCaptureDefinition);
	RelevantAttributesToCapture.Add(DefenseCaptureDefinition);
	RelevantAttributesToCapture.Add(CriticalChanceCaptureDefinition);
	RelevantAttributesToCapture.Add(CriticalDamageMultiplierCaptureDefinition);
}

void UAssassinDamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	
	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();
	
	float AttackPower = 0.0f;
	float Defense = 0.0f;
	float CriticalChance = 0.0f;
	float CriticalDamageMultiplier = 1.0f;
	
	const bool bCapturedAttackPower = ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(AttackPowerCaptureDefinition, EvaluationParameters, AttackPower);
	const bool bCapturedDefense = ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DefenseCaptureDefinition, EvaluationParameters, Defense);
	const bool bCapturedCriticalChance = ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CriticalChanceCaptureDefinition, EvaluationParameters, CriticalChance);
	const bool bCapturedCriticalDamageMultiplier = ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(CriticalDamageMultiplierCaptureDefinition, EvaluationParameters, CriticalDamageMultiplier);
	
	if (!bCapturedAttackPower || !bCapturedDefense || !bCapturedCriticalChance || !bCapturedCriticalDamageMultiplier)
	{
		UE_LOG(LogTemp, Error, TEXT("[Execution][Damage] Attribute Capture Failed | Attack=%d Defense=%d CritChance=%d CritMultiplier=%d"), bCapturedAttackPower, bCapturedDefense, bCapturedCriticalChance, bCapturedCriticalDamageMultiplier);
		return;
	}

	AttackPower = FMath::Max(AttackPower, 0.0f);
	Defense = FMath::Max(Defense, 0.0f);
	CriticalChance = FMath::Clamp(CriticalChance, 0.0f, 100.0f);
	CriticalDamageMultiplier = FMath::Max(CriticalDamageMultiplier, 1.0f);

	const float BaseDamage = FMath::Max(AttackPower - Defense, 0.0f);
	const float CriticalRoll = static_cast<float>(FMath::RandHelper(10000)) / 100.0f;
	const bool bIsCritical = CriticalRoll < CriticalChance;
	const float FinalDamage = BaseDamage * (bIsCritical ? CriticalDamageMultiplier : 1.0f);
	
	OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(UAssassinAttributeSet::GetHealthAttribute(), EGameplayModOp::Additive, -FinalDamage));

	UE_LOG(LogTemp, Display, TEXT("[Execution][Damage] AttackPower=%.1f | Defense=%.1f | BaseDamage=%.1f | CritChance=%.1f%% | CritRoll=%.2f | Critical=%d | CritMultiplier=%.2f | FinalDamage=%.1f"), AttackPower, Defense, BaseDamage, CriticalChance, CriticalRoll, bIsCritical, CriticalDamageMultiplier, FinalDamage);
}

UAssassinDamageEffect::UAssassinDamageEffect()
{
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayEffectExecutionDefinition& Execution = Executions.AddDefaulted_GetRef();
	Execution.CalculationClass = UAssassinDamageExecution::StaticClass();
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