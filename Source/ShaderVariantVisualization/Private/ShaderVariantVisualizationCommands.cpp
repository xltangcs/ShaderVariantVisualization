// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShaderVariantVisualizationCommands.h"

#define LOCTEXT_NAMESPACE "FShaderVariantVisualizationModule"

void FShaderVariantVisualizationCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "SVV", "Bring up ShaderVariantVisualization window", EUserInterfaceActionType::Button, FInputGesture());
}

#undef LOCTEXT_NAMESPACE
