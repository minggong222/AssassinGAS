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
#include "AIController.h"
#include "BrainComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "PhysicsEngine/PhysicsAsset.h"

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

void AAssassinCharacterBase::HandleDeath()
{
	if (bDeathHandled)
	{
		return;
	}
	
	bDeathHandled = true;
	
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();

		if (UBrainComponent* BrainComponent = AIController->GetBrainComponent())
		{
			BrainComponent->StopLogic(TEXT("Character Died"));
		}

		AIController->ClearFocus(EAIFocusPriority::Gameplay);
		AIController->ClearFocus(EAIFocusPriority::Move);
		AIController->ClearFocus(EAIFocusPriority::Default);
		AIController->SetControlRotation(GetActorRotation());
	}

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	
	if (UCharacterMovementComponent* Movement = GetCharacterMovement())
	{
		Movement->bOrientRotationToMovement = false;
		Movement->bUseControllerDesiredRotation = false;
		Movement->StopMovementImmediately();
		Movement->DisableMovement();
	}

	if (DeathAnimation && GetMesh())
	{
		bDeathAnimationPending = true;
		GetMesh()->PlayAnimation(DeathAnimation, false);

		if (!GetMesh()->IsPlaying())
		{
			bDeathAnimationPending = false;
			ActivateRagdoll();
		}

		UE_LOG(LogTemp, Display, TEXT("[Character][Death] Animation Started | Character=%s | Animation=%s"), *GetName(), *DeathAnimation->GetName());
	}
	else
	{
		ActivateRagdoll();
	}

	UE_LOG(LogTemp, Warning, TEXT("[Character][Death] Movement and AI Stopped | Character=%s"), *GetName());
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

	if (!bDeathAnimationPending || !GetMesh())
	{
		return;
	}

	if (GetMesh()->IsPlaying())
	{
		return;
	}

	bDeathAnimationPending = false;
	ActivateRagdoll();
}

// Called to bind functionality to input
void AAssassinCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AAssassinCharacterBase::ActivateRagdoll()
{
	if (bRagdollActivated)
	{
		return;
	}

	USkeletalMeshComponent* SkeletalMesh = GetMesh();

	if (!SkeletalMesh)
	{
		return;
	}

	if (!SkeletalMesh->GetPhysicsAsset())
	{
		UE_LOG(LogTemp, Error, TEXT("[Character][Death] Physics Asset Missing | Character=%s"), *GetName());
		return;
	}

	bRagdollActivated = true;

	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	SkeletalMesh->SetCollisionProfileName(TEXT("Ragdoll"));
	SkeletalMesh->SetAllBodiesSimulatePhysics(true);
	SkeletalMesh->SetSimulatePhysics(true);
	SkeletalMesh->WakeAllRigidBodies();

	UE_LOG(LogTemp, Display, TEXT("[Character][Death] Ragdoll Activated | Character=%s"), *GetName());
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
