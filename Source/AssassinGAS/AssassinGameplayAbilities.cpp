
#include "AssassinGameplayAbilities.h"
#include "AssassinAttributeSet.h"
#include "AssassinGameplayEffects.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "AssassinGameplayTags.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "CollisionShape.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "CoreGlobals.h"

UAssassinAttackAbility::UAssassinAttackAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    FGameplayTagContainer AssetTags;
    AssetTags.AddTag(AssassinGameplayTags::Ability_Attack);
    SetAssetTags(AssetTags);
    ActivationOwnedTags.AddTag(AssassinGameplayTags::State_Attacking);

    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Dead);
    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Stunned);
    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Sprinting);
    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Dodging);
}

void UAssassinAttackAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    AActor* AvatarActor = ActorInfo ? ActorInfo->AvatarActor.Get() : nullptr;
    UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
    
    if (!AvatarActor || !SourceASC || !AttackMontage)
    {
        const UAssassinAttackAbility* DefaultAbility = GetClass()->GetDefaultObject<UAssassinAttackAbility>();
        UE_LOG(LogTemp, Error, TEXT("[GA_Attack] Invalid Reference | Class=%s RuntimeMontage=%s DefaultMontage=%s"), *GetClass()->GetPathName(), *GetNameSafe(AttackMontage), *GetNameSafe(DefaultAbility->AttackMontage));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    HitActors.Reset();
    LastAttackTraceFrame = 0;
    bHasAttackTraceFrame = false;

    UAbilityTask_WaitGameplayEvent* EventTask = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, AssassinGameplayTags::Event_Attack_Hit, nullptr, false, true);
    UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, AttackMontage, 1.0f);
    
    if (!EventTask || !MontageTask)
    {
        UE_LOG(LogTemp, Error, TEXT("[GA_Attack] Failed to create AbilityTask | EventTask=%d MontageTask=%d"), EventTask != nullptr, MontageTask != nullptr);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }
    
    EventTask->EventReceived.AddDynamic(this, &UAssassinAttackAbility::OnAttackTraceEvent);
    MontageTask->OnCompleted.AddDynamic(this, &UAssassinAttackAbility::OnAttackMontageCompleted);
    MontageTask->OnInterrupted.AddDynamic(this, &UAssassinAttackAbility::OnAttackMontageInterrupted);
    MontageTask->OnCancelled.AddDynamic(this, &UAssassinAttackAbility::OnAttackMontageCancelled);
    
    EventTask->ReadyForActivation();
    MontageTask->ReadyForActivation();
}

void UAssassinAttackAbility::OnAttackTraceEvent(FGameplayEventData Payload)
{
    AActor* AvatarActor = GetAvatarActorFromActorInfo();
    UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
    
    if (!IsActive() || !AvatarActor || !SourceASC || !AvatarActor->GetWorld())
    {
        UE_LOG(LogTemp, Error, TEXT("[GA_Attack] Attack Trace Invalid Reference"));
        return;
    }

    if (bHasAttackTraceFrame && LastAttackTraceFrame == GFrameCounter)
    {
        return;
    }

    LastAttackTraceFrame = GFrameCounter;
    bHasAttackTraceFrame = true;

    const FVector TraceStart = AvatarActor->GetActorLocation() + FVector(0.0f,0.0f,45.0f);
    const FVector TraceEnd = TraceStart + AvatarActor->GetActorForwardVector() * 180.0f;
    const FCollisionShape AttackShape = FCollisionShape::MakeSphere(65.0f);
    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(AssassinAttack), false, AvatarActor);
    
    TArray<FHitResult> HitResults;
    const bool bHit = AvatarActor->GetWorld()->SweepMultiByChannel(HitResults, TraceStart, TraceEnd, FQuat::Identity, ECC_Pawn, AttackShape, QueryParams);
    
    if (!bHit)
    {
        return;
    }

    for (const FHitResult& Hit : HitResults)
    {
        AActor* TargetActor = Hit.GetActor();

        if (!TargetActor || TargetActor == AvatarActor)
        {
            continue;
        }

        const IAbilitySystemInterface* TargetInterface = Cast<IAbilitySystemInterface>(TargetActor);
        UAbilitySystemComponent* TargetASC = TargetInterface ? TargetInterface->GetAbilitySystemComponent() : nullptr;

        if (!TargetASC)
        {
            continue;
        }
        
        const TWeakObjectPtr<AActor> TargetReference(TargetActor);

        if (HitActors.Contains(TargetReference))
        {
            continue;
        }
        
        if (TargetASC->HasMatchingGameplayTag(AssassinGameplayTags::State_Dead))
        {
            continue;
        }

        FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
        EffectContext.AddSourceObject(AvatarActor);

        FGameplayEffectSpecHandle DamageSpec = SourceASC->MakeOutgoingSpec(UAssassinDamageEffect::StaticClass(), 1.0f, EffectContext);

        if (DamageSpec.IsValid())
        {
            SourceASC->ApplyGameplayEffectSpecToTarget(*DamageSpec.Data.Get(), TargetASC);
            HitActors.Add(TargetReference);

            const UAssassinAttributeSet* TargetAttributes = TargetASC->GetSet<UAssassinAttributeSet>();

            if (TargetAttributes)
            {
                UE_LOG(LogTemp, Display, TEXT("[GA_Attack] Hit | Target=%s | Health=%.1f/%.1f"), *TargetActor->GetName(), TargetAttributes->GetHealth(), TargetAttributes->GetMaxHealth());
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[GA_Attack] Failed to create DamageEffect Spec"));
        }

        break;
    }
}


void UAssassinAttackAbility::OnAttackMontageCompleted()
{
    if (IsActive())
    {
        EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
    }
}

void UAssassinAttackAbility::OnAttackMontageInterrupted()
{
    UE_LOG(LogTemp, Warning, TEXT("[GA_Attack] Montage Interrupted"));

    if (IsActive())
    {
        EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
    }
}

void UAssassinAttackAbility::OnAttackMontageCancelled()
{
    UE_LOG(LogTemp, Warning, TEXT("[GA_Attack] Montage Cancelled"));

    if (IsActive())
    {
        EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
    }
}


UAssassinHealAbility::UAssassinHealAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    
    FGameplayTagContainer AssetTags;
    AssetTags.AddTag(AssassinGameplayTags::Ability_Heal);
    SetAssetTags(AssetTags);
    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Dead);
    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Stunned);
    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Dodging);
}

void UAssassinHealAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

    if (ASC)
    {
        FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
        EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());

        FGameplayEffectSpecHandle HealSpec = ASC->MakeOutgoingSpec(UAssassinHealEffect::StaticClass(), 1.0f, EffectContext);

        if (HealSpec.IsValid())
        {
            ASC->ApplyGameplayEffectSpecToSelf(*HealSpec.Data.Get());
        }
    }

    EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}

UAssassinSprintAbility::UAssassinSprintAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    FGameplayTagContainer AssetTags;
    AssetTags.AddTag(AssassinGameplayTags::Ability_Sprint);
    SetAssetTags(AssetTags);
    ActivationOwnedTags.AddTag(AssassinGameplayTags::State_Sprinting);

    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Dead);
    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Stunned);
    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Attacking);
    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Dodging);
}

void UAssassinSprintAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    const UAssassinAttributeSet* Attributes = ASC ? ASC->GetSet<UAssassinAttributeSet>() : nullptr;
    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());

    if (!ASC || !Attributes || !Character || Attributes->GetStamina() < 15.0f || !CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
    EffectContext.AddSourceObject(Character);

    FGameplayEffectSpecHandle CostSpec = ASC->MakeOutgoingSpec(UAssassinStaminaCostEffect::StaticClass(), 1.0f, EffectContext);

    if (!CostSpec.IsValid())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ASC->ApplyGameplayEffectSpecToSelf(*CostSpec.Data.Get());

    UCharacterMovementComponent* Movement = Character->GetCharacterMovement();

    if (Movement)
    {
        OriginalMaxWalkSpeed = Movement->MaxWalkSpeed;
        Movement->MaxWalkSpeed = OriginalMaxWalkSpeed * 1.5f;
    }
}

void UAssassinSprintAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    if (ActorInfo && OriginalMaxWalkSpeed > 0.0f)
    {
        ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get());

        if (Character && Character->GetCharacterMovement())
        {
            Character->GetCharacterMovement()->MaxWalkSpeed = OriginalMaxWalkSpeed;
        }
    }

    OriginalMaxWalkSpeed = 0.0f;

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}


UAssassinJumpAbility::UAssassinJumpAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    FGameplayTagContainer AssetTags;
    AssetTags.AddTag(AssassinGameplayTags::Ability_Jump);
    SetAssetTags(AssetTags);
    ActivationOwnedTags.AddTag(AssassinGameplayTags::State_Jumping);

    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Dead);
    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Stunned);
    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Attacking);
    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Jumping);
    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Dodging);
}

void UAssassinJumpAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());

    if (!Character || !Character->CanJump() || !CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        UE_LOG(LogTemp, Warning, TEXT("[GA_Jump] Activation Failed | CharacterValid=%d | CanJump=%d"), Character != nullptr, Character ? Character->CanJump() : false);
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    Character->Jump();

    UE_LOG(LogTemp, Display, TEXT("[GA_Jump] Activated | JumpZVelocity=%.1f | JumpCount=%d/%d | VelocityZ=%.1f"),
        Character->GetCharacterMovement()->JumpZVelocity,
        Character->JumpCurrentCount,
        Character->JumpMaxCount,
        Character->GetVelocity().Z);
}

void UAssassinJumpAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;

    if (Character)
    {
        Character->StopJumping();
    }

    UE_LOG(LogTemp, Display, TEXT("[GA_Jump] Ended | Cancelled=%d"), bWasCancelled);

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

UAssassinDodgeAbility::UAssassinDodgeAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    
    FGameplayTagContainer AssetTags;
    AssetTags.AddTag(AssassinGameplayTags::Ability_Dodge);
    SetAssetTags(AssetTags);
    ActivationOwnedTags.AddTag(AssassinGameplayTags::State_Dodging);
    
    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Dead);
    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Stunned);
    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Attacking);
    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Sprinting);
    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Jumping);
    ActivationBlockedTags.AddTag(AssassinGameplayTags::State_Dodging);
}


void UAssassinDodgeAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

    ACharacter* Character = Cast<ACharacter>(GetAvatarActorFromActorInfo());
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    const UAssassinAttributeSet* Attributes = ASC ? ASC->GetSet<UAssassinAttributeSet>() : nullptr;
    UCharacterMovementComponent* Movement = Character ? Character->GetCharacterMovement() : nullptr;
    UAnimInstance* AnimInstance = Character && Character->GetMesh() ? Character->GetMesh()->GetAnimInstance() : nullptr;

    if (!Character || !ASC || !Attributes || !Movement || !AnimInstance || !DodgeMontage)
    {
        UE_LOG(LogTemp, Error, TEXT("[GA_Dodge] Invalid Reference | Character=%d ASC=%d Attributes=%d Movement=%d AnimInstance=%d Montage=%d"),
            Character != nullptr,
            ASC != nullptr,
            Attributes != nullptr,
            Movement != nullptr,
            AnimInstance != nullptr,
            DodgeMontage != nullptr);

        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const float StaminaBefore = Attributes->GetStamina();

    if (!Movement->IsMovingOnGround() || StaminaBefore < 15.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("[GA_Dodge] Activation Failed | Grounded=%d Stamina=%.1f Required=15.0"),
            Movement->IsMovingOnGround(),
            StaminaBefore);

        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        UE_LOG(LogTemp, Warning, TEXT("[GA_Dodge] CommitAbility Failed"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
    EffectContext.AddSourceObject(Character);

    FGameplayEffectSpecHandle CostSpec = ASC->MakeOutgoingSpec(UAssassinStaminaCostEffect::StaticClass(), 1.0f, EffectContext);

    if (!CostSpec.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("[GA_Dodge] Failed to create Stamina Cost Spec"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    FVector DodgeDirection = Character->GetLastMovementInputVector();
    DodgeDirection.Z = 0.0f;

    if (DodgeDirection.IsNearlyZero())
    {
        DodgeDirection = Character->GetActorForwardVector();
        DodgeDirection.Z = 0.0f;
    }

    DodgeDirection = DodgeDirection.GetSafeNormal();
    Character->SetActorRotation(DodgeDirection.Rotation());

    UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(this, NAME_None, DodgeMontage, 1.0f);

    if (!MontageTask)
    {
        UE_LOG(LogTemp, Error, TEXT("[GA_Dodge] Failed to create Montage Task"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    ASC->ApplyGameplayEffectSpecToSelf(*CostSpec.Data.Get());

    MontageTask->OnCompleted.AddDynamic(this, &UAssassinDodgeAbility::OnDodgeMontageCompleted);
    MontageTask->OnInterrupted.AddDynamic(this, &UAssassinDodgeAbility::OnDodgeMontageInterrupted);
    MontageTask->OnCancelled.AddDynamic(this, &UAssassinDodgeAbility::OnDodgeMontageCancelled);
    MontageTask->ReadyForActivation();

    UE_LOG(LogTemp, Display, TEXT("[GA_Dodge] Montage Started | Montage=%s | Direction=%s | RootMotion=%d | Stamina=%.1f -> %.1f | Playing=%d"),
        *DodgeMontage->GetName(),
        *DodgeDirection.ToString(),
        DodgeMontage->bEnableRootMotionTranslation,
        StaminaBefore,
        Attributes->GetStamina(),
        AnimInstance->Montage_IsPlaying(DodgeMontage));
}

void UAssassinDodgeAbility::OnDodgeMontageCompleted()
{
    UE_LOG(LogTemp, Display, TEXT("[GA_Dodge] Montage Completed"));

    if (IsActive())
    {
        EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, false);
    }
}

void UAssassinDodgeAbility::OnDodgeMontageInterrupted()
{
    UE_LOG(LogTemp, Warning, TEXT("[GA_Dodge] Montage Interrupted"));

    if (IsActive())
    {
        EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
    }
}

void UAssassinDodgeAbility::OnDodgeMontageCancelled()
{
    UE_LOG(LogTemp, Warning, TEXT("[GA_Dodge] Montage Cancelled"));

    if (IsActive())
    {
        EndAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true, true);
    }
}

void UAssassinDodgeAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    const UAssassinAttributeSet* Attributes = ASC ? ASC->GetSet<UAssassinAttributeSet>() : nullptr;
    ACharacter* Character = ActorInfo ? Cast<ACharacter>(ActorInfo->AvatarActor.Get()) : nullptr;

    const FVector EndLocation = Character ? Character->GetActorLocation() : FVector::ZeroVector;

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

    UE_LOG(LogTemp, Display, TEXT("[GA_Dodge] Ended | Cancelled=%d | Dodging=%d | Stamina=%.1f | EndLocation=%s"),
        bWasCancelled,
        ASC ? ASC->GetGameplayTagCount(AssassinGameplayTags::State_Dodging) : 0,
        Attributes ? Attributes->GetStamina() : 0.0f,
        *EndLocation.ToString());
}