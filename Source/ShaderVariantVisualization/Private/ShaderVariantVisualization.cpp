// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShaderVariantVisualization.h"

#include "IMaterialEditor.h"
#include "ShaderVariantVisualizationStyle.h"
#include "ShaderVariantVisualizationCommands.h"
#include "LevelEditor.h"
#include "MaterialEditorModule.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"
#include "Materials/MaterialInstance.h"
#include "ShaderVariantWidget.h"

#include "ShaderVariant.h"

static const FName ShaderVariantVisualizationTabName("ShaderVariantVisualization");

#define LOCTEXT_NAMESPACE "FShaderVariantVisualizationModule"


FShaderVariantVisualizationModule::FShaderVariantVisualizationModule()
	:	CurrentMaterialEditorPtr(nullptr),
		CurrentMaterialInterfacePtr(nullptr)
{
	IMaterialEditorModule& MaterialEditorModule = FModuleManager::LoadModuleChecked<IMaterialEditorModule>("MaterialEditor");
	MaterialEditorModule.OnMaterialInstanceEditorOpened().AddRaw(this, &FShaderVariantVisualizationModule::OnMaterialInstanceEditorOpened);
	MaterialEditorModule.OnMaterialEditorOpened().AddRaw(this, &FShaderVariantVisualizationModule::OnMaterialEditorOpened);
}

FShaderVariantVisualizationModule::~FShaderVariantVisualizationModule()
{
	IMaterialEditorModule& MaterialEditorModule = FModuleManager::LoadModuleChecked<IMaterialEditorModule>("MaterialEditor");
	MaterialEditorModule.OnMaterialInstanceEditorOpened().RemoveAll(this);
	MaterialEditorModule.OnMaterialEditorOpened().RemoveAll(this);

	
}

void FShaderVariantVisualizationModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FShaderVariantVisualizationStyle::Initialize();
	FShaderVariantVisualizationStyle::ReloadTextures();

	FShaderVariantVisualizationCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FShaderVariantVisualizationCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FShaderVariantVisualizationModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FShaderVariantVisualizationModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(ShaderVariantVisualizationTabName, FOnSpawnTab::CreateRaw(this, &FShaderVariantVisualizationModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FShaderVariantVisualizationTabTitle", "ShaderVariantVisualization"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FShaderVariantVisualizationModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FShaderVariantVisualizationStyle::Shutdown();

	FShaderVariantVisualizationCommands::Unregister();

	if(FGlobalTabmanager::Get()->HasTabSpawner(ShaderVariantVisualizationTabName))
	{
		FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ShaderVariantVisualizationTabName);
	}
}

TSharedRef<SDockTab> FShaderVariantVisualizationModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	if(!CurrentMaterialInterfacePtr)
	{
		return SNew(SDockTab)
			.TabRole(ETabRole::NomadTab)
			[
				// Put your tab content here!
				SNew(SBox)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("There isn't material or material instance available now!")))
				]
			];
	}
	
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SShaderVariantWidget)
			.ShaderVariantPtr(ShaderVariant)
		];
}



void FShaderVariantVisualizationModule::OnMaterialInstanceEditorOpened(TWeakPtr<IMaterialEditor> MaterialEditor)
{
	if (MaterialEditor.IsValid()) 
	{
		TSharedPtr<IMaterialEditor> MaterialEditorSharedPtr = MaterialEditor.Pin();
		
		CurrentMaterialEditorPtr = MaterialEditorSharedPtr.Get();
		
		CurrentMaterialEditorPtr->OnMaterialEditorClosed().AddRaw(this, &FShaderVariantVisualizationModule::OnMaterialInstanceEditorClosed);
	}
}

void FShaderVariantVisualizationModule::OnMaterialEditorOpened(TWeakPtr<IMaterialEditor> MaterialEditor)
{
	if (MaterialEditor.IsValid()) 
	{
		TSharedPtr<IMaterialEditor> MaterialEditorSharedPtr = MaterialEditor.Pin();
		
		CurrentMaterialEditorPtr = MaterialEditorSharedPtr.Get();
		
		CurrentMaterialEditorPtr->OnMaterialEditorClosed().AddRaw(this, &FShaderVariantVisualizationModule::OnMaterialInstanceEditorClosed);
	}
}

void FShaderVariantVisualizationModule::OnMaterialInstanceEditorClosed()
{
	if(SvvSDockTab.IsValid())
	{
		SvvSDockTab->RequestCloseTab();
	}
}


void FShaderVariantVisualizationModule::AddToolBarExtension(FToolBarBuilder& Builder)
{
	Builder.BeginSection(TEXT("Plugins"));
	Builder.AddToolBarButton(FShaderVariantVisualizationCommands::Get().OpenPluginWindow);
	Builder.EndSection();
}

void FShaderVariantVisualizationModule::AddMenuBarExtension(FMenuBarBuilder& Builder)
{
	Builder.AddMenuEntry(FShaderVariantVisualizationCommands::Get().OpenPluginWindow);
}

void FShaderVariantVisualizationModule::PluginButtonClicked()
{
	CurrentMaterialInterfacePtr = CurrentMaterialEditorPtr->GetMaterialInterface();
	
	ShaderVariant = new FShaderVariant(CurrentMaterialInterfacePtr);
	
	SvvSDockTab = FGlobalTabmanager::Get()->FindExistingLiveTab(ShaderVariantVisualizationTabName);

	if (!SvvSDockTab.IsValid())
	{
		SvvSDockTab = FGlobalTabmanager::Get()->TryInvokeTab(ShaderVariantVisualizationTabName);
	}
	else
	{
		FGlobalTabmanager::Get()->DrawAttention(SvvSDockTab.ToSharedRef());
	}
}

void FShaderVariantVisualizationModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	/*{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FShaderVariantVisualizationCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FShaderVariantVisualizationCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}*/
	
	{
		IMaterialEditorModule& MaterialEditorModule = FModuleManager::LoadModuleChecked<IMaterialEditorModule>("MaterialEditor");
		/*IMaterialEditorModule& MaterialEditorModule = IMaterialEditorModule::Get();*/
		{
			TSharedRef<FExtender>  MenuExtender = MakeShareable(new FExtender);
			MenuExtender->AddToolBarExtension("Parent", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FShaderVariantVisualizationModule::AddToolBarExtension));
			MaterialEditorModule.GetToolBarExtensibilityManager()->AddExtender(MenuExtender);
		}
	}
	{
		IMaterialEditorModule& MaterialEditorModule = FModuleManager::LoadModuleChecked<IMaterialEditorModule>("MaterialEditor");
		/*IMaterialEditorModule& MaterialEditorModule = IMaterialEditorModule::Get();*/
		{
			TSharedRef<FExtender>  MenuExtender = MakeShareable(new FExtender);
			MenuExtender->AddMenuBarExtension("Help", EExtensionHook::After, PluginCommands, FMenuBarExtensionDelegate::CreateRaw(this, &FShaderVariantVisualizationModule::AddMenuBarExtension));
			MaterialEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FShaderVariantVisualizationModule, ShaderVariantVisualization)