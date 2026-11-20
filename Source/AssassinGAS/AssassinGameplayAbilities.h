// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "AssassinGameplayAbilities.generated.h"

class UAnimMontage;
class AActor;
/**
 * 
 */

UCLASS()
class ASSASSINGAS_API UAssassinAttackAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAssassinAttackAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
protected:
	UFUNCTION()
	void OnAttackMontageCompleted();

	UFUNCTION()
	void OnAttackMontageInterrupted();

	UFUNCTION()
	void OnAttackMontageCancelled();

	UFUNCTION()
	void OnAttackTraceEvent(FGameplayEventData Payload);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	TSet<TWeakObjectPtr<AActor>> HitActors;
	uint64 LastAttackTraceFrame = 0;
	bool bHasAttackTraceFrame = false;
};

UCLASS()
class ASSASSINGAS_API UAssassinHealAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAssassinHealAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
};

UCLASS()
class ASSASSINGAS_API UAssassinSprintAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAssassinSprintAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
	float OriginalMaxWalkSpeed = 0.0f;
};


UCLASS()
class ASSASSINGAS_API UAssassinJumpAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAssassinJumpAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
};



UCLASS()
class ASSASSINGAS_API UAssassinDodgeAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UAssassinDodgeAbility();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	UFUNCTION()
	void OnDodgeMontageCompleted();

	UFUNCTION()
	void OnDodgeMontageInterrupted();

	UFUNCTION()
	void OnDodgeMontageCancelled();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> DodgeMontage;
};
