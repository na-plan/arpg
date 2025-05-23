#pragma once

#include "GameInstance/NAItemGameInstanceSubsystem.h"

struct ARPG_API FItemManagerStatics
{
	static FItemManagerImpl& Get(const UWorld* InWorld);
};
