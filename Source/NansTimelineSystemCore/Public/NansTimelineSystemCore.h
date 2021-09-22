// Copyright Nans Pellicari, 2021

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/** Required to create a UE4 module */
class FNansTimelineSystemCoreModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	/** IModuleInterface implementation */
	virtual void ShutdownModule() override;
};
