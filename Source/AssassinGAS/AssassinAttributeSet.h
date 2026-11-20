// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AssassinAttributeSet.generated.h"

#define ASSASSIN_ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
    GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

/**
 * 
 */
UCLASS()
class ASSASSINGAS_API UAssassinAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	UAssassinAttributeSet();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Health;
	ASSASSIN_ATTRIBUTE_ACCESSORS(UAssassinAttributeSet, Health)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ASSASSIN_ATTRIBUTE_ACCESSORS(UAssassinAttributeSet, MaxHealth)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Stamina;
	ASSASSIN_ATTRIBUTE_ACCESSORS(UAssassinAttributeSet, Stamina)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData MaxStamina;
	ASSASSIN_ATTRIBUTE_ACCESSORS(UAssassinAttributeSet, MaxStamina)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData AttackPower;
	ASSASSIN_ATTRIBUTE_ACCESSORS(UAssassinAttributeSet, AttackPower)
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Defense;
	ASSASSIN_ATTRIBUTE_ACCESSORS(UAssassinAttributeSet, Defense)
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData CriticalChance;
	ASSASSIN_ATTRIBUTE_ACCESSORS(UAssassinAttributeSet, CriticalChance)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData CriticalDamageMultiplier;
	ASSASSIN_ATTRIBUTE_ACCESSORS(UAssassinAttributeSet, CriticalDamageMultiplier)
protected:
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
private:
	void ClampAttributeValue(const FGameplayAttribute& Attribute, float& NewValue) const;
};
