#include "ShaderVariantWidget.h"

#include "ShaderVariant.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "IMaterialEditor.h"
#include "MaterialEditor/Private/MaterialEditor.h"
#include "MaterialGraph/MaterialGraphNode.h"
#include "MaterialGraph/MaterialGraphNode_Root.h"
#include "Subsystems/AssetEditorSubsystem.h"

#define PADDING_SIZE 4
#define SPACER_SIZE FVector2D(0.2, 0.2)
SShaderVariantWidget::SShaderVariantWidget()
{
}

SShaderVariantWidget::~SShaderVariantWidget()
{
	delete ShaderVariant;
}

void SShaderVariantWidget::Construct(const FArguments& Args)
{
	ShaderVariant = Args._ShaderVariantPtr;
	ChildSlot
	[
		SAssignNew(TopScrollBox, SScrollBox)
	];
	
	GetWidgetFromVertexFactoryTypeNameMap(Args._ShaderVariantPtr->GetVertexFactoryTypeNameMap());
	GetWidgetFromShaderTypeNameField(Args._ShaderVariantPtr->GetShaderTypeNameField());
	GetWidgetFromStaticSwitchParameter(Args._ShaderVariantPtr->GetStaticSwitchParametersMap());
}

void SShaderVariantWidget::GetWidgetFromVertexFactoryTypeNameMap(TMap<FName, TArray<FName>>& VertexFactoryTypeNameMap) const
{
	TSharedPtr<SScrollBox> VertexFactoryTypeNameMapWidget;
	
	TopScrollBox->AddSlot()
	.Padding(PADDING_SIZE)
	.HAlign(HAlign_Fill)
	[
		SNew(SExpandableArea)
		.InitiallyCollapsed(true)
		.BorderImage( FEditorStyle::GetBrush( "ToolBar.Background" ) )
		.Padding(PADDING_SIZE)
		.HeaderContent()
		[
		  SNew(STextBlock )
		  
		  .Text(FText::FromName("Vertex Factory Type"))
		]
		.BodyContent()
		[
			SAssignNew(VertexFactoryTypeNameMapWidget, SScrollBox)
		]
	];
	
	for(const auto& Element : VertexFactoryTypeNameMap)
	{
		TSharedPtr<SScrollBox> SubScrollBox;
		VertexFactoryTypeNameMapWidget->AddSlot()
		[
			SNew(SExpandableArea)
			.InitiallyCollapsed(true)
			.BorderImage( FEditorStyle::GetBrush( "ToolBar.Background" ) )
			.Padding(PADDING_SIZE)
			.HeaderContent()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.Padding(PADDING_SIZE)
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Text(FText::FromName(Element.Key))
				]
				+ SHorizontalBox::Slot()
				.Padding(PADDING_SIZE)
				.HAlign(HAlign_Left)
				[
					SNew(SSpacer)
					.Size(SPACER_SIZE)
				]
				+SHorizontalBox::Slot()
				.Padding(PADDING_SIZE)
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Text(FText::AsNumber(Element.Value.Num()))
				]
			]
			.BodyContent()
			[
				SAssignNew(SubScrollBox, SScrollBox)
			]
		];

		for(const auto& ItemData : Element.Value)
		{
			SubScrollBox->AddSlot()
			.Padding(PADDING_SIZE)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.Padding(PADDING_SIZE)
				.HAlign(HAlign_Left)
				[
					SNew(STextBlock)
					.Text(FText::FromName(ItemData))
					.Margin(FMargin(20,0,0,0))
				]
			];
		}
	}
}

void SShaderVariantWidget::GetWidgetFromShaderTypeNameField(FShaderTypeNameField* ShaderTypeNameField) 
{
	TSharedPtr<SScrollBox> ShaderTypeNameFieldWidget;

	TopScrollBox->AddSlot()
	.Padding(PADDING_SIZE)
	[
		SNew(SExpandableArea)
		.InitiallyCollapsed(true)
		.BorderImage( FEditorStyle::GetBrush( "ToolBar.Background" ) )
		.Padding(PADDING_SIZE)
		.HeaderContent()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.Padding(PADDING_SIZE)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock )
				.Text(FText::FromName("Shader Type"))
			]
			+ SHorizontalBox::Slot()
			.Padding(PADDING_SIZE)
			.HAlign(HAlign_Left)
			[
				SNew(SSpacer)
				.Size(SPACER_SIZE)
			]
			+SHorizontalBox::Slot()
			.Padding(PADDING_SIZE)
			.HAlign(HAlign_Left)
			[
				SNew(STextBlock)
				.Text(FText::AsNumber(ShaderTypeNameField->VariantNum))
			]
		]
		.BodyContent()
		[
			SAssignNew(ShaderTypeNameFieldWidget, SScrollBox)
		]
	];
	
	AddMapToScrollBox(ShaderTypeNameField->ShaderTypeNameChildrenField, ShaderTypeNameFieldWidget);
	
}

void SShaderVariantWidget::GetWidgetFromStaticSwitchParameter(TMap<FName, TPair<UObject*, UMaterialExpression*>>& StaticSwitchParametersMap)
{
	TSharedPtr<SScrollBox> StaticSwitchParameterWidget;

	TopScrollBox->AddSlot()
	.Padding(PADDING_SIZE)
	[
		SNew(SExpandableArea)
		.InitiallyCollapsed(true)
		.BorderImage( FEditorStyle::GetBrush( "ToolBar.Background" ) )
		.Padding(PADDING_SIZE)
		.HeaderContent()
		[
		  SNew(STextBlock )
		  .Text(FText::FromName("Static Switch Parameter"))
		]
		.BodyContent()
		[
			SAssignNew(StaticSwitchParameterWidget, SScrollBox)
		]
	];

	for(const auto& ParameterPair : StaticSwitchParametersMap)
	{
		StaticSwitchParameterWidget->AddSlot()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(PADDING_SIZE)
			[
				SNew(SBox)
				.WidthOverride(20) // 设置缩进大小，这里设置为20像素
				[
					SNew(SSpacer)
				]
			]
			+SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(PADDING_SIZE)
			[
				SNew(SButton)
				.Text(FText::FromName(ParameterPair.Key))
				.OnClicked(this, &SShaderVariantWidget::OnStaticSwitchParameterButtonClicked, ParameterPair.Value)
			]
		];
	}
}

FReply SShaderVariantWidget::OnStaticSwitchParameterButtonClicked(TPair<UObject*, UMaterialExpression*> InMaterialExpression)
{
	UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>();
	AssetEditorSubsystem->OpenEditorForAsset(InMaterialExpression.Key);

	if (IMaterialEditor* MaterialEditor = (IMaterialEditor*)AssetEditorSubsystem->FindEditorForAsset(InMaterialExpression.Key, true))
	{
		FMaterialEditor* MaterialEditorInstance = (FMaterialEditor*)MaterialEditor;
		
		// Attention：GetGraphEditor() native engine does not support, added in MaterialEditor.h line 588
		for(auto& Node: MaterialEditorInstance->GetGraphEditor()->GetCurrentGraph()->Nodes)
		{
			if(Node->IsA<UMaterialGraphNode_Root>()) continue;
			
			const UMaterialGraphNode* GraphNode = Cast<UMaterialGraphNode>(Node);
			if(GraphNode->MaterialExpression->GetMaterialExpressionId() == InMaterialExpression.Value->GetMaterialExpressionId())
			{
				InMaterialExpression.Value->GraphNode = Node;
				MaterialEditorInstance->JumpToExpression(InMaterialExpression.Value);
				return FReply::Handled();
			}
		}
	}
	
	return FReply::Handled();
}

void SShaderVariantWidget::AddMapToScrollBox(TMap<FString, FShaderTypeNameField*>& ShaderTypeNameChildrenField,
                                             TSharedPtr<SScrollBox>& ScrollBox, int TreeLevel)
{
	if(ShaderTypeNameChildrenField.Num() != 0)
	{
		for(auto& Element : ShaderTypeNameChildrenField)
		{
			if(Element.Value->ShaderTypeNameChildrenField.Num() == 0)
			{
				ScrollBox->AddSlot()
				.Padding(PADDING_SIZE)
				[
					SNew(STextBlock)
					.Text(FText::FromString(Element.Key))
					.Margin(FMargin(10* TreeLevel,0,0,0))
				];
				
				continue;
			}
			
			TSharedPtr<SScrollBox> SubScrollBox;
			ScrollBox->AddSlot()
			.Padding(PADDING_SIZE)
			[
				SNew(SExpandableArea)
				.InitiallyCollapsed(true)
				.BorderImage( FEditorStyle::GetBrush( "ToolBar.Background" ) )
				.Padding(PADDING_SIZE)
				.HeaderContent()
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.Padding(PADDING_SIZE)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.Text(FText::FromString(Element.Key))
					]
					+ SHorizontalBox::Slot()
					.Padding(PADDING_SIZE)
					.HAlign(HAlign_Left)
					[
						SNew(SSpacer)
						.Size(SPACER_SIZE)
					]
					+SHorizontalBox::Slot()
					.Padding(PADDING_SIZE)
					.HAlign(HAlign_Left)
					[
						SNew(STextBlock)
						.Text(FText::AsNumber(Element.Value->VariantNum))
					]
				]
				.BodyContent()
				[
					SAssignNew(SubScrollBox, SScrollBox)
				]
			];
			
			AddMapToScrollBox(Element.Value->ShaderTypeNameChildrenField, SubScrollBox, TreeLevel + 1);
		}
	}
}
