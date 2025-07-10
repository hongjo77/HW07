#include "PlayerControllerHW.h"
#include "EnhancedInputSubsystems.h"
#include "PawnHW.h"
#include "DroneHW.h"

APlayerControllerHW::APlayerControllerHW()
	: InputmappingContext(nullptr),
	MoveAction(nullptr),
	LookAction(nullptr)
{

}
void APlayerControllerHW::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->ClearAllMappings();

			if (Cast<ADroneHW>(InPawn))
			{
				Subsystem->AddMappingContext(DroneInputmappingContext, 0);
			}
			if (Cast<APawnHW>(InPawn))
			{
				Subsystem->AddMappingContext(InputmappingContext, 0);
			}
		}
	}
}