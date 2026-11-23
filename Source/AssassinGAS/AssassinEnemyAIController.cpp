#include "AssassinEnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Navigation/PathFollowingComponent.h"
#include "AssassinGASCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "AbilitySystemComponent.h"
#include "AssassinCharacterBase.h"
#include "AssassinGameplayTags.h"

namespace
{
	constexpr float HiddenDetectionRadius = 350.0f;
}

AAssassinEnemyAIController::AAssassinEnemyAIController()
{
	UAIPerceptionComponent* AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("EnemyAIPerception"));
	SetPerceptionComponent(*AIPerception);

	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 1500.0f;
	SightConfig->LoseSightRadius = 1800.0f;
	SightConfig->PeripheralVisionAngleDegrees = 100.0f;

	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->SetDominantSense(UAISense_Sight::StaticClass());
}

void AAssassinEnemyAIController::RefreshPlayerStealthState(AAssassinGASCharacter* Player)
{
	if (!Player)
	{
		return;
	}

	UAIPerceptionComponent* AIPerception = GetAIPerceptionComponent();

	if (!AIPerception)
	{
		return;
	}

	FActorPerceptionBlueprintInfo PerceptionInfo;

	if (!AIPerception->GetActorsPerception(Player, PerceptionInfo))
	{
		return;
	}

	for (const FAIStimulus& Stimulus : PerceptionInfo.LastSensedStimuli)
	{
		if (Stimulus.WasSuccessfullySensed())
		{
			OnTargetPerceptionUpdated(Player, Stimulus);
			return;
		}
	}
}

void AAssassinEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (!BehaviorTreeAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("[EnemyAI] BehaviorTree Asset is Missing | Controller=%s"), *GetName());
		return;
	}

	if (!RunBehaviorTree(BehaviorTreeAsset))
	{
		UE_LOG(LogTemp, Error, TEXT("[EnemyAI] Failed to Run BehaviorTree | Controller=%s"), *GetName());
		return;
	}

	if (UAIPerceptionComponent* AIPerception = GetAIPerceptionComponent())
	{
		AIPerception->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &AAssassinEnemyAIController::OnTargetPerceptionUpdated);
	}

	UE_LOG(LogTemp, Display, TEXT("[EnemyAI] BehaviorTree Started | Controller=%s | Pawn=%s"), *GetName(), *GetNameSafe(InPawn));
}

void AAssassinEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	AAssassinCharacterBase* Enemy = Cast<AAssassinCharacterBase>(GetPawn());
	UAbilitySystemComponent* EnemyASC = Enemy ? Enemy->GetAbilitySystemComponent() : nullptr;

	if (!EnemyASC || EnemyASC->HasMatchingGameplayTag(AssassinGameplayTags::State_Dead))
	{
		return;
	}

	AAssassinGASCharacter* Player = Cast<AAssassinGASCharacter>(Actor);
	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();

	if (!IsValid(Player) || !BlackboardComponent)
	{
		return;
	}

	UAbilitySystemComponent* PlayerASC = Player->GetAbilitySystemComponent();
	const bool bPlayerHidden = PlayerASC && PlayerASC->HasMatchingGameplayTag(AssassinGameplayTags::State_Hidden);

	if (bPlayerHidden && GetPawn() && FVector::Dist2D(GetPawn()->GetActorLocation(), Player->GetActorLocation()) > HiddenDetectionRadius)
	{
		if (BlackboardComponent->GetValueAsObject(TEXT("TargetActor")) == Player)
		{
			const FVector LastKnownLocation = Player->GetActorLocation();
			BlackboardComponent->SetValueAsVector(TEXT("LastKnownLocation"), LastKnownLocation);
			BlackboardComponent->SetValueAsBool(TEXT("CanSeeTarget"), false);
			SetFocalPoint(LastKnownLocation, EAIFocusPriority::Gameplay);

			UE_LOG(LogTemp, Display, TEXT("[EnemyAI][Stealth] Player Hidden | Player=%s | Distance=%.1f"), *Player->GetName(), FVector::Dist2D(GetPawn()->GetActorLocation(), Player->GetActorLocation()));
		}

		return;
	}

	if (Stimulus.WasSuccessfullySensed())
	{
		BlackboardComponent->SetValueAsObject(TEXT("TargetActor"), Player);
		BlackboardComponent->SetValueAsVector(TEXT("LastKnownLocation"), Player->GetActorLocation());
		BlackboardComponent->SetValueAsBool(TEXT("CanSeeTarget"), true);

		SetFocus(Player, EAIFocusPriority::Gameplay);

		UE_LOG(LogTemp, Display, TEXT("[EnemyAI][Sight] Detected | Target=%s | Location=%s"), *Player->GetName(), *Player->GetActorLocation().ToString());
	}
	else
	{
		if (BlackboardComponent->GetValueAsObject(TEXT("TargetActor")) != Player)
		{
			return;
		}

		BlackboardComponent->SetValueAsBool(TEXT("CanSeeTarget"), false);

		const FVector LastKnownLocation = BlackboardComponent->GetValueAsVector(TEXT("LastKnownLocation"));
		SetFocalPoint(LastKnownLocation, EAIFocusPriority::Gameplay);

		UE_LOG(LogTemp, Display, TEXT("[EnemyAI][Sight] LostSight | Target=%s | LastKnown=%s"), *Player->GetName(), *LastKnownLocation.ToString());
	}
}