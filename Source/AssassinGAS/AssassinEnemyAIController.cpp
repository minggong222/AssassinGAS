#include "AssassinEnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Navigation/PathFollowingComponent.h"
#include "AssassinGASCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"

AAssassinEnemyAIController::AAssassinEnemyAIController()
{
	PerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>("AIPerceptionComponent");
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("SightConfig");
	
	SightConfig->SightRadius = 1500.0f;
	SightConfig->LoseSightRadius = 1800.0f;
	SightConfig->PeripheralVisionAngleDegrees = 70.0f;
	
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	
	PerceptionComponent->ConfigureSense(*SightConfig);
	PerceptionComponent->SetDominantSense(UAISense_Sight::StaticClass());
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

	if (PerceptionComponent)
	{
		PerceptionComponent->OnTargetPerceptionUpdated.AddUniqueDynamic(this, &AAssassinEnemyAIController::OnTargetPerceptionUpdated);
	}

	UE_LOG(LogTemp, Display, TEXT("[EnemyAI] BehaviorTree Started | Controller=%s | Pawn=%s"), *GetName(), *GetNameSafe(InPawn));
}

void AAssassinEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	AAssassinGASCharacter* Player = Cast<AAssassinGASCharacter>(Actor);
	UBlackboardComponent* Blackboard = GetBlackboardComponent();
	
	if (!IsValid(Player) || !Blackboard)
	{
		return;
	}
	
	if (Stimulus.WasSuccessfullySensed())
	{
		Blackboard->SetValueAsObject(TEXT("TargetActor"), Player);
		Blackboard->SetValueAsVector(TEXT("LastKnownLocation"), Player->GetActorLocation());
		Blackboard->SetValueAsBool(TEXT("CanSeeTarget"), true);
		
		UE_LOG(LogTemp, Display, TEXT("[EnemyAI][Sight] Detected | Target=%s | Location=%s"), *Player->GetName(), *Player->GetActorLocation().ToString());
	}
	else
	{
		if (Blackboard->GetValueAsObject(TEXT("TargetActor")) != Player)
		{
			return;
		}
		
		Blackboard->SetValueAsBool(TEXT("CanSeeTarget"), false);
		
		UE_LOG(LogTemp, Display, TEXT("[EnemyAI][Sight] LostSight | Target=%s | LastKnown=%s"), *Player->GetName(), *Blackboard->GetValueAsVector(TEXT("LastKnownLocation")).ToString());
	}
}
