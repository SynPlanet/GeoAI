// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FGeoJsonWrapperModule : public IModuleInterface
{
public:
        /**
        * @brief Initializes the module when the game or editor starts.
        *
        * This method is called by the engine to start the module. It should be used to perform any initialization tasks required by the module,
        * such as registering components, setting up data structures, or initializing any necessary
        * variables.
        */
        virtual void StartupModule() override;

        /**
        * @brief Shuts down the GeoJsonWrapper module.
        *
        * This function is called when the module is unloaded.
        * It should perform any necessary cleanup and shutdown.
        */
        virtual void ShutdownModule() override;
};