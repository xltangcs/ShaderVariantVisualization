// Copyright Epic Games, Inc. All Rights Reserved.

#include "ShaderVariantVisualization.h"
#include "ShaderVariantVisualizationStyle.h"
#include "ShaderVariantVisualizationCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

#include "IMaterialEditor.h"
#include "EditorStyleSet.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SExpandableArea.h"

static const FName ShaderVariantVisualizationTabName("ShaderVariantVisualization");
static const FName ShaderVariantVisualizationTabNameAbbreviation("SVV");

#define LOCTEXT_NAMESPACE "FShaderVariantVisualizationModule"

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
		.SetDisplayName(LOCTEXT("FShaderVariantVisualizationTabTitle", "Shader Variant"))
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

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(ShaderVariantVisualizationTabName);
}

TSharedRef<SDockTab> FShaderVariantVisualizationModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	FButtonStyle ButtonStyle = FButtonStyle()
	.SetNormal(FSlateColorBrush(FColor::Red)) // 设置正常状态下的颜色为红色
	.SetHovered(FSlateColorBrush(FColor::Green)) // 设置鼠标悬停状态下的颜色为绿色
	.SetPressed(FSlateColorBrush(FColor::Blue)); // 设置按下状态下的颜色为蓝色

	FText WidgetText = FText::Format(
		LOCTEXT("WindowWidgetText", "Add code to {0} in {1} to override this window's contents"),
		FText::FromString(TEXT("FShaderVariantVisualizationModule::OnSpawnPluginTab")),
		FText::FromString(TEXT("ShaderVariantVisualization.cpp"))
		);

	TSharedPtr<SDockTab> DetailsTab = SNew(SDockTab)
		.Icon( FEditorStyle::GetBrush("LevelEditor.Tabs.Details") )
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			/*.SlotPadding(FMargin(0))
			.SizeParam( FSizeParam(FSizeParam::SizeRule_Auto, 0.0f) )*/
			.Padding(2)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.OnClicked(FOnClicked::CreateRaw(this, &FShaderVariantVisualizationModule::ButtonClicked))
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Test Text1")))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2)
			[
				SNew(SButton)
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				.OnClicked(FOnClicked::CreateRaw(this, &FShaderVariantVisualizationModule::ButtonClicked))
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Test Text2")))
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(2)
			[
				SNew(SExpandableArea)
				.InitiallyCollapsed(false)
				.BorderImage( FEditorStyle::GetBrush( "ToolBar.Background" ) )
				.Padding(8.0f)
				.HeaderContent()
				[
				  SNew( STextBlock )
				  .Text( NSLOCTEXT("MyWidget","CategoryHeader", "My Awesome Category") )
				]
				.BodyContent()
				[
					// ... Whatever content widgets ...
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(2)
					[
				
						SNew(SBorder)
						.Padding(2)
						.Content()
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Test Text3")))
						]
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					.Padding(2)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.OnClicked(FOnClicked::CreateRaw(this, &FShaderVariantVisualizationModule::ButtonClicked))
						[
							SNew(STextBlock)
							.Text(FText::FromString(TEXT("Test Text4")))
						]
					]
				]
			]
		];
	
	return DetailsTab.ToSharedRef();
}

void FShaderVariantVisualizationModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(ShaderVariantVisualizationTabName);
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
		IMaterialEditorModule& MaterialEditor = FModuleManager::LoadModuleChecked<IMaterialEditorModule>("MaterialEditor");
		{
			TSharedRef<FExtender>  MenuExtender = MakeShareable(new FExtender);
			MenuExtender->AddMenuBarExtension("Help", EExtensionHook::After, PluginCommands, FMenuBarExtensionDelegate::CreateRaw(this, &FShaderVariantVisualizationModule::AddMenuExtension));
			MaterialEditor.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
		}
	}
}

FReply FShaderVariantVisualizationModule::ButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Hello, Button!"));
	return FReply::Handled();
}

void FShaderVariantVisualizationModule::AddMenuExtension(FMenuBarBuilder& Builder)
{
	//Builder.BeginSection(TEXT("MyButton"));
	Builder.AddMenuEntry(FShaderVariantVisualizationCommands::Get().OpenPluginWindow);
	//Builder.EndSection();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FShaderVariantVisualizationModule, ShaderVariantVisualization)