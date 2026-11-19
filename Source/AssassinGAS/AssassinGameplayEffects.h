// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffect.h"
#include "AssassinGameplayEffects.generated.h"

/**
 * 
 */
UCLASS()
class ASSASSINGAS_API UAssassinDamageEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UAssassinDamageEffect();
};

UCLASS()
class ASSASSINGAS_API UAssassinHealEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UAssassinHealEffect();
};

UCLASS()
class ASSASSINGAS_API UAssassinStaminaCostEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UAssassinStaminaCostEffect();
};

UCLASS()
class ASSASSINGAS_API UAssassinStaminaRestoreEffect : public UGameplayEffect
{
	GENERATED_BODY()

public:
	UAssassinStaminaRestoreEffect();
};