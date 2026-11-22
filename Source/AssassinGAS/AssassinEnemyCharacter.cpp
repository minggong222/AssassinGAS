// Fill out your copyright notice in the Description page of Project Settings.


#include "AssassinEnemyCharacter.h"
#include "AIController.h"
#include "Components/SkeletalMeshComponent.h"

AAssassinEnemyCharacter::AAssassinEnemyCharacter()
{
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	GetMesh()->SetCanEverAffectNavigation(false);
}