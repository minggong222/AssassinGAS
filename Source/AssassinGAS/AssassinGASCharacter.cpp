// Copyright Epic Games, Inc. All Rights Reserved.

#include "AssassinGASCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "AssassinGAS.h"
#include "AssassinGameplayAbilities.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "InputCoreTypes.h"
#include "AssassinGameplayTags.h"
#include "AssassinAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

void AAssassinGASCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	if (!HasAuthority() || bAbilitiesGranted)
	{
		return;
	}
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	
	if (!ASC)
	{
		return;
	}
	
	ASC->GiveAbility(FGameplayAbilitySpec(UAssassinAttackAbility::StaticClass(), 1));
	ASC->GiveAbility(FGameplayAbilitySpec(UAssassinHealAbility::StaticClass(), 1));
	ASC->GiveAbility(FGameplayAbilitySpec(UAssassinSprintAbility::StaticClass(), 1));
	ASC->GiveAbility(FGameplayAbilitySpec(UAssassinJumpAbility::StaticClass(), 1));
	ASC->GiveAbility(FGameplayAbilitySpec(UAssassinDodgeAbility::StaticClass(), 1));
	
	bAbilitiesGranted = true;
}

AAssassinGASCharacter::AAssassinGASCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}

void AAssassinGASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AAssassinGASCharacter::InputJumpStarted);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AAssassinGASCharacter::InputJumpEnded);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAssassinGASCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &AAssassinGASCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AAssassinGASCharacter::Look);
	}
	else
	{
		UE_LOG(LogAssassinGAS, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
	PlayerInputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AAssassinGASCharacter::InputAttack);
	PlayerInputComponent->BindKey(EKeys::H, IE_Pressed, this, &AAssassinGASCharacter::InputHeal);
	PlayerInputComponent->BindKey(EKeys::LeftShift, IE_Pressed, this, &AAssassinGASCharacter::InputSprintStarted);
	PlayerInputComponent->BindKey(EKeys::LeftShift, IE_Released, this, &AAssassinGASCharacter::InputSprintEnded);
	PlayerInputComponent->BindKey(EKeys::LeftControl, IE_Pressed, this, &AAssassinGASCharacter::InputDodge);
}

void AAssassinGASCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void AAssassinGASCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void AAssassinGASCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void AAssassinGASCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void AAssassinGASCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void AAssassinGASCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}


void AAssassinGASCharacter::InputAttack()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();

	if (!ASC)
	{
		return;
	}

	const bool bActivated = ASC->TryActivateAbilityByClass(UAssassinAttackAbility::StaticClass(), true);

	UE_LOG(LogTemp, Display, TEXT("[Input][Attack] Activated=%d | Dead=%d | Stunned=%d | Sprinting=%d"),
		bActivated,
		ASC->GetGameplayTagCount(AssassinGameplayTags::State_Dead),
		ASC->GetGameplayTagCount(AssassinGameplayTags::State_Stunned),
		ASC->GetGameplayTagCount(AssassinGameplayTags::State_Sprinting));
}

void AAssassinGASCharacter::InputHeal()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();

	if (!ASC)
	{
		return;
	}

	const bool bActivated = ASC->TryActivateAbilityByClass(UAssassinHealAbility::StaticClass(), true);
	const UAssassinAttributeSet* Attributes = ASC->GetSet<UAssassinAttributeSet>();

	if (Attributes)
	{
		UE_LOG(LogTemp, Display, TEXT("[Input][Heal] Activated=%d | Health=%.1f/%.1f | Dead=%d"),
			bActivated,
			Attributes->GetHealth(),
			Attributes->GetMaxHealth(),
			ASC->GetGameplayTagCount(AssassinGameplayTags::State_Dead));
	}
}

void AAssassinGASCharacter::InputSprintStarted()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();

	if (!ASC)
	{
		return;
	}

	const bool bActivated = ASC->TryActivateAbilityByClass(UAssassinSprintAbility::StaticClass(), true);
	const UAssassinAttributeSet* Attributes = ASC->GetSet<UAssassinAttributeSet>();

	UE_LOG(LogTemp, Display, TEXT("[Input][Sprint] Activated=%d | Stamina=%.1f | Dead=%d | Attacking=%d | Sprinting=%d"),
		bActivated,
		Attributes ? Attributes->GetStamina() : 0.0f,
		ASC->GetGameplayTagCount(AssassinGameplayTags::State_Dead),
		ASC->GetGameplayTagCount(AssassinGameplayTags::State_Attacking),
		ASC->GetGameplayTagCount(AssassinGameplayTags::State_Sprinting));
}

void AAssassinGASCharacter::InputSprintEnded()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();

	if (!ASC)
	{
		return;
	}

	FGameplayAbilitySpec* SprintSpec = ASC->FindAbilitySpecFromClass(UAssassinSprintAbility::StaticClass());

	if (SprintSpec)
	{
		ASC->CancelAbilityHandle(SprintSpec->Handle);
	}

	UE_LOG(LogTemp, Display, TEXT("[Input][Sprint] Released | Sprinting=%d | MaxWalkSpeed=%.1f"),
		ASC->GetGameplayTagCount(AssassinGameplayTags::State_Sprinting),
		GetCharacterMovement()->MaxWalkSpeed);
}


void AAssassinGASCharacter::InputJumpStarted()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();

	if (!ASC)
	{
		return;
	}

	const bool bActivated = ASC->TryActivateAbilityByClass(UAssassinJumpAbility::StaticClass(), true);

	UE_LOG(LogTemp, Display, TEXT("[Input][Jump] Activated=%d | Jumping=%d | CanJump=%d"),
		bActivated,
		ASC->GetGameplayTagCount(AssassinGameplayTags::State_Jumping),
		CanJump());
}

void AAssassinGASCharacter::InputJumpEnded()
{
	StopJumping();

	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();

	UE_LOG(LogTemp, Display, TEXT("[Input][Jump] Released | Jumping=%d | VelocityZ=%.1f"),
		ASC ? ASC->GetGameplayTagCount(AssassinGameplayTags::State_Jumping) : 0,
		GetVelocity().Z);
}

void AAssassinGASCharacter::InputDodge()
{
	UAbilitySystemComponent* ASC = GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}
	
	const bool bActivated = ASC->TryActivateAbilityByClass(UAssassinDodgeAbility::StaticClass(), true);
	const UAssassinAttributeSet* Attributes = ASC->GetSet<UAssassinAttributeSet>();
	
	UE_LOG(LogTemp, Display, TEXT("[Input][Dodge] Activated=%d | Stamina=%.1f | Dodging=%d | Jumping=%d | Dead=%d"),
		bActivated,
		Attributes ? Attributes->GetStamina() : 0.0f,
		ASC->GetGameplayTagCount(AssassinGameplayTags::State_Dodging),
		ASC->GetGameplayTagCount(AssassinGameplayTags::State_Jumping),
		ASC->GetGameplayTagCount(AssassinGameplayTags::State_Dead));
}
