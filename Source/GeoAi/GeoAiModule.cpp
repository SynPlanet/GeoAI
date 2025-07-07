// Copyright Epic Games, Inc. All Rights Reserved.

#include "GeoAiModule.h"
#include "Modules/ModuleManager.h"

class FGeoAiModule : public FDefaultGameModuleImpl
{
        virtual void StartupModule() override {}

        virtual void ShutdownModule() override {}
};

IMPLEMENT_GAME_MODULE(FGeoAiModule, GeoAi);

DEFINE_LOG_CATEGORY(LogGeoAi);
