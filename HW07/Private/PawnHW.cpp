#include "PawnHW.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include <Components/CapsuleComponent.h>
#include <PlayerControllerHW.h>

APawnHW::APawnHW()
{
	PrimaryActorTick.bCanEverTick = true;

	RootComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
	Cast<UCapsuleComponent>(RootComponent)->SetSimulatePhysics(false);

	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	SkeletalMeshComp->SetupAttachment(RootComponent);
	SkeletalMeshComp->SetSimulatePhysics(false);

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.0f;
	SpringArmComp->bUsePawnControlRotation = false;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;

	Speed = 400.0f;
	Yaw = 0.0f;
	YawSpeed = 1.0f;
	Pitch = 0.0f;
	PitchSpeed = 1.0f;
}

void APawnHW::BeginPlay()
{
	Super::BeginPlay();
	FRotator SpringArmRotation = SpringArmComp->GetRelativeRotation();
	Pitch = SpringArmRotation.Pitch;
	Yaw = SpringArmRotation.Yaw;
	NearbyDrone = nullptr;
}

void APawnHW::Move(const FInputActionValue& value)
{
	if (!Controller) return;

	const FVector2D MoveInput = value.Get<FVector2D>();
	if (MoveInput.IsNearlyZero()) return;

	FVector Forward = GetActorForwardVector();
	FVector Right = GetActorRightVector();

	FVector Moveto = (Forward * MoveInput.Y + Right * MoveInput.X).GetSafeNormal();
	
	AddActorWorldOffset(Moveto * Speed * GetWorld()->GetDeltaSeconds(), true);
}

void APawnHW::Look(const FInputActionValue& value)
{
	const FVector2D LookInput = value.Get<FVector2D>();
	if (LookInput.IsNearlyZero()) return;

	Pitch = FMath::Clamp(Pitch + LookInput.Y * PitchSpeed, -80.0f, 80.0f);

	FRotator DeltaYaw(0.f, LookInput.X * YawSpeed, 0.f);
	AddActorLocalRotation(DeltaYaw);

	SpringArmComp->SetRelativeRotation(FRotator(Pitch, 0.f, 0.f));
}

void APawnHW::TryInteract(const FInputActionValue& value)
{
	if (NearbyDrone)
	{
		if (APlayerControllerHW* PC = Cast<APlayerControllerHW>(GetController()))
		{
			PC->Possess(NearbyDrone);
			GEngine->AddOnScreenDebugMessage(
				-1,
				2.0f,
				FColor::Green,
				FString::Printf(TEXT("Start ride Drone")));
		}
	}
}

void APawnHW::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (APlayerControllerHW* PlayerController = Cast<APlayerControllerHW>(GetController()))
		{
			if (PlayerController->MoveAction)
			{
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&APawnHW::Move
				);
			}
			if (PlayerController->LookAction)
			{
				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&APawnHW::Look
				);
			}
			if (PlayerController->InteractAction)
			{
				EnhancedInput->BindAction(
					PlayerController->InteractAction,
					ETriggerEvent::Triggered,
					this,
					&APawnHW::TryInteract
				);
			}
		}
	}
}

