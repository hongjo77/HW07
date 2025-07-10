#include "DroneHW.h"
#include "EnhancedInputComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include <Components/CapsuleComponent.h>
#include <Components/SphereComponent.h>
#include <PlayerControllerHW.h>
#include <PawnHW.h>

ADroneHW::ADroneHW()
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

	InteractCollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("InteractCollision"));
	InteractCollisionComp->InitSphereRadius(250.0f);
	InteractCollisionComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	InteractCollisionComp->SetupAttachment(RootComponent);
	InteractCollisionComp->OnComponentBeginOverlap.AddDynamic(this, &ADroneHW::OnOverlapBegin);
	InteractCollisionComp->OnComponentEndOverlap.AddDynamic(this, &ADroneHW::OnOverlapEnd);

	bPlayerNearby = false;
	CachedPlayer = nullptr;
	MoveSpeed = 1500.0f;
	RotationSpeed = 150.0f;
	Velocity = FVector::ZeroVector;
	bIsGround = true;
	bPrevIsGround = true;
	Gravity = -980.0f;
}

void ADroneHW::BeginPlay()
{
	Super::BeginPlay();
}

void ADroneHW::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsGround)
	{
		Velocity.Z += Gravity * DeltaTime;
		Velocity.Z = FMath::Clamp(Velocity.Z, Gravity, 0.f);
	}

	FVector Delta = Velocity * DeltaTime;
	AddActorWorldOffset(Delta, true);

	float CapsuleHalfHeight = 0.0f;
	if (UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(RootComponent))
	{
		CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
	}

	FVector Start = GetActorLocation();
	FVector End = Start - FVector(0, 0, CapsuleHalfHeight+ 3.0f);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_Visibility, Params
	);

	bPrevIsGround = bIsGround;
	bIsGround = bHit;

	if (bIsGround && !bPrevIsGround)
	{
		Velocity.Z = 0.0f;
	}
}

void ADroneHW::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (DroneMoveAction)
		{
			EnhancedInput->BindAction(
				DroneMoveAction,
				ETriggerEvent::Triggered,
				this,
				&ADroneHW::Move3D
			);
		}
		if (LookAction)
		{
			EnhancedInput->BindAction(
				LookAction,
				ETriggerEvent::Triggered,
				this,
				&ADroneHW::Look
			);
		}
		if (RollAction)
		{
			EnhancedInput->BindAction(
				RollAction,
				ETriggerEvent::Triggered,
				this,
				&ADroneHW::Roll
			);
		}
	}
}

void ADroneHW::Move3D(const FInputActionValue& value)
{
	const FVector MoveInput = value.Get<FVector>();
	if (MoveInput.IsNearlyZero()) return;

	FVector MoveVec =
		GetActorForwardVector() * MoveInput.X +
		GetActorRightVector() * MoveInput.Y +
		GetActorUpVector() * MoveInput.Z;

	AddActorWorldOffset(MoveVec * MoveSpeed * GetWorld()->GetDeltaSeconds(), true);
}
void ADroneHW::Look(const FInputActionValue& value)
{
	const FVector2D LookInput = value.Get<FVector2D>();
	if (LookInput.IsNearlyZero()) return;
	
	FRotator DeltaRotation(0.f, 0.f, 0.f);
	DeltaRotation.Yaw = LookInput.X;
	DeltaRotation.Pitch = LookInput.Y;

	AddActorLocalRotation(FRotator(DeltaRotation.Pitch, DeltaRotation.Yaw, 0.f));
}

void ADroneHW::Roll(const FInputActionValue& value)
{
	float RollInput = value.Get<float>();
	if (FMath::IsNearlyZero(RollInput)) return;

	float DeltaTime = GetWorld()->GetDeltaSeconds();
	AddActorLocalRotation(FRotator(0.f, 0.f, RollInput * RotationSpeed * DeltaTime));
}

void ADroneHW::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this && OtherActor->IsA(APawnHW::StaticClass()))
	{
		bPlayerNearby = true;
		CachedPlayer = OtherActor;
		GEngine->AddOnScreenDebugMessage(
			-1,
			2.0f,
			FColor::Green,
			FString::Printf(TEXT("Press E button to get Drone")));
		if (APawnHW* PlayerPawn = Cast<APawnHW>(OtherActor))
		{
			PlayerPawn->NearbyDrone = this;
		}
	}
}

void ADroneHW::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherActor == CachedPlayer)
	{
		bPlayerNearby = false;
		CachedPlayer = nullptr;
		if (APawnHW* PlayerPawn = Cast<APawnHW>(OtherActor))
		{
			PlayerPawn->NearbyDrone = nullptr;
		}
	}
}
