// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "ShaderVariantVisualizationStyle.h"

class FShaderVariantVisualizationCommands : public TCommands<FShaderVariantVisualizationCommands>
{
public:

	FShaderVariantVisualizationCommands()
		: TCommands<FShaderVariantVisualizationCommands>(TEXT("ShaderVariantVisualization"), NSLOCTEXT("Contexts", "ShaderVariantVisualization", "ShaderVariantVisualization Plugin"), NAME_None, FShaderVariantVisualizationStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};