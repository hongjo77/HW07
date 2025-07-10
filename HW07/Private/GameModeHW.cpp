#include "GameModeHW.h"
#include "PawnHW.h"
#include "PlayerControllerHW.h"

AGameModeHW::AGameModeHW()
{
	DefaultPawnClass = APawnHW::StaticClass();
	PlayerControllerClass = APlayerControllerHW::StaticClass();
}
