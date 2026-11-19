// Fill out your copyright notice in the Description page of Project Settings.


#include "AssassinEnemyCharacter.h"

#include "AssassinGameplayAbilities.h"
#include "AssassinGameplayTags.h"
#include "AssassinGASCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

AAssassinEnemyCharacter::AAssassinEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	
	BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	BodyMesh->SetupAttachment(GetCapsuleComponent());
	BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (CubeMesh.Succeeded())
	{
		BodyMesh->SetStaticMesh(CubeMesh.Object);
	}

	BodyMesh->SetRelativeScale3D(FVector(0.65f, 0.45f, 0.9f));

	HeadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(GetCapsuleComponent());
	HeadMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (SphereMesh.Succeeded())
	{
		HeadMesh->SetStaticMesh(SphereMesh.Object);
	}

	HeadMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 65.0f));
	HeadMesh->SetRelativeScale3D(FVector(0.36f));
}