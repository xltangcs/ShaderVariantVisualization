// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class FToolBarBuilder;
class FMenuBuilder;

class FShaderVariantVisualizationModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command (by default it will bring up plugin window) */
	void PluginButtonClicked();
	
private:

	void RegisterMenus();

	TSharedRef<class SDockTab> OnSpawnPluginTab(const class FSpawnTabArgs& SpawnTabArgs);
	TSharedPtr<class FUICommandList> PluginCommands;
	
public:
	FShaderVariantVisualizationModule();
	virtual ~FShaderVariantVisualizationModule() override;

	// Event
	void OnMaterialInstanceEditorOpened(TWeakPtr<class IMaterialEditor> MaterialEditor);
	void OnMaterialEditorOpened(TWeakPtr<class IMaterialEditor> MaterialEditor);
	void OnMaterialInstanceEditorClosed();
	
private:
	void AddToolBarExtension(class FToolBarBuilder& Builder);
	void AddMenuBarExtension(class FMenuBarBuilder& Builder);
	
	class IMaterialEditor* CurrentMaterialEditorPtr;
	class UMaterialInterface* CurrentMaterialInterfacePtr;
	TSharedPtr<SDockTab> SvvSDockTab;

	class FShaderVariant* ShaderVariant;
	
};
