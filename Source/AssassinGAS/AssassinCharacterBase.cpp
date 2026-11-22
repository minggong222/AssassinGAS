// Fill out your copyright notice in the Description page of Project Settings.


#include "AssassinCharacterBase.h"
#include "AssassinAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AssassinGameplayEffects.h"
#include "GameplayEffect.h"
#include "AssassinGameplayTags.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"

// Sets default values
AAssassinCharacterBase::AAssassinCharacterBase()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UAssassinAttributeSet>(TEXT("AttributeSet"));
}

UAbilitySystemComponent* AAssassinCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

bool AAssassinCharacterBase::ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level)
{
	if (!AbilitySystemComponent || !EffectClass)
	{
		return false;
	}
	
	FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	
	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(EffectClass, Level, EffectContext);
	
	if (!SpecHandle.IsValid())
	{
		return false;
	}
	
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	return true;
}

// Called when the game starts or when spawned
void AAssassinCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	InitializeAbilityActorInfo();
}

void AAssassinCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitializeAbilityActorInfo();
	GrantDefaultAbilities();
}

void AAssassinCharacterBase::OnRep_Controller()
{
	Super::OnRep_Controller();
	InitializeAbilityActorInfo();
}


void AAssassinCharacterBase::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (!AbilitySystemComponent)
	{
		return;
	}

	FGameplayTagContainer JumpAbilityTags;
	JumpAbilityTags.AddTag(AssassinGameplayTags::Ability_Jump);
	
	const int32 JumpingTagBefore = AbilitySystemComponent->GetGameplayTagCount(AssassinGameplayTags::State_Jumping);
	const bool bJumpAbilityActive = JumpingTagBefore > 0;
	
	if (bJumpAbilityActive)
	{
		AbilitySystemComponent->CancelAbilities(&JumpAbilityTags, nullptr, nullptr);
	}

	UE_LOG(LogTemp, Display, TEXT("[Character][Landed] JumpActive=%d | JumpingTag=%d -> %d | CanJump=%d"),
		bJumpAbilityActive,
		JumpingTagBefore,
		AbilitySystemComponent->GetGameplayTagCount(AssassinGameplayTags::State_Jumping),
		CanJump());
}


// Called every frame
void AAssassinCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AAssassinCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AAssassinCharacterBase::GrantDefaultAbilities()
{
	if (!HasAuthority() || !AbilitySystemComponent || bDefaultAbilitiesGranted)
	{
		return;
	}
	
	int32 GrantedCount = 0;
	
	for (const TSubclassOf<UGameplayAbility> AbilityClass : DefaultAbilities)
	{
		if (!AbilityClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("[GAS][GrantAbility] Invalid Ability Class | Character=%s"), *GetName());
			continue;
		}
		
		const FGameplayAbilitySpecHandle AbilityHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1));
		
		if (AbilityHandle.IsValid())
		{
			++GrantedCount;
		}
		
		UE_LOG(LogTemp, Display, TEXT("[GAS][GrantAbility] Character=%s | Ability=%s | Granted=%d"),
		   *GetName(),
		   *GetNameSafe(AbilityClass.Get()),
		   AbilityHandle.IsValid());
	}
	
	bDefaultAbilitiesGranted = true;
	
	UE_LOG(LogTemp, Display, TEXT("[GAS][GrantDefaultAbilities] Character=%s | Configured=%d | Granted=%d"),
	   *GetName(),
	   DefaultAbilities.Num(),
	   GrantedCount);
}

void AAssassinCharacterBase::InitializeAbilityActorInfo()
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}
