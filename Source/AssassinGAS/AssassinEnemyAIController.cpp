#include "AssassinEnemyAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Navigation/PathFollowingComponent.h"

AAssassinEnemyAIController::AAssassinEnemyAIController()
{
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

	UE_LOG(LogTemp, Display, TEXT("[EnemyAI] BehaviorTree Started | Controller=%s | Pawn=%s"), *GetName(), *GetNameSafe(InPawn));
}

void AAssassinEnemyAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	(void)RequestID;

	UE_LOG(LogTemp, Warning, TEXT("[EnemyAI][MoveCompleted] Pawn=%s | Result=%s | Flags=%u"), *GetNameSafe(GetPawn()), *Result.ToString(), static_cast<uint32>(Result.Flags));
}