// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AssassinCharacterBase.generated.h"


class UAbilitySystemComponent;
class UAssassinAttributeSet;
class UGameplayEffect;
class UGameplayAbility;
class AController;
class UAnimSequence;

UCLASS()
class ASSASSINGAS_API AAssassinCharacterBase : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AAssassinCharacterBase();
	
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	bool ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level = 1.0f);
	virtual void HandleDeath();
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;
	virtual void Landed(const FHitResult& Hit) override;
	
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Abilities")
	TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "Animation|Death")
	TObjectPtr<UAnimSequence> DeathAnimation;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "GAS")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(VisibleAnywhere, Category = "GAS")
	TObjectPtr<UAssassinAttributeSet> AttributeSet;
	
	bool bDefaultAbilitiesGranted = false;
	bool bDeathHandled = false;
	bool bDeathAnimationPending = false;
	bool bRagdollActivated = false;

	void ActivateRagdoll();
	
	void GrantDefaultAbilities();
	void InitializeAbilityActorInfo();
};
