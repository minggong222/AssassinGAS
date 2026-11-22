// Fill out your copyright notice in the Description page of Project Settings.


#include "AssassinEnemyCharacter.h"
#include "AIController.h"
AAssassinEnemyCharacter::AAssassinEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	
	AIControllerClass = AAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}