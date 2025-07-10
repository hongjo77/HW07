#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "DroneHW.h"
#include "PawnHW.generated.h"

class USpringArmComponent;
class UCameraComponent;
struct FInputActionValue;

UCLASS()
class HW07_API APawnHW : public APawn
{
	GENERATED_BODY()

public:
	APawnHW();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMeshComponent* SkeletalMeshComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArmComp;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CameraComp;
	UPROPERTY()
	ADroneHW* NearbyDrone;

	
protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void Move(const FInputActionValue& value);
	UFUNCTION()
	void Look(const FInputActionValue& value);
	UFUNCTION()
	void TryInteract(const FInputActionValue& value);
	

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	float Speed;
	float Yaw;
	float YawSpeed;
	float Pitch;
	float PitchSpeed;
};
