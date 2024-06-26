#pragma once

#include "CoreMinimal.h"

class SScrollBox;
class FShaderVariant;
struct FShaderTypeNameField;

class SShaderVariantWidget : public SCompoundWidget
{
public:
	SShaderVariantWidget();
	virtual ~SShaderVariantWidget() override;
	
	using FShaderVariantPtr = FShaderVariant*;
	
	SLATE_BEGIN_ARGS(SShaderVariantWidget) {}
		SLATE_ARGUMENT(FShaderVariantPtr, ShaderVariantPtr)
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);

private:
	void GetWidgetFromVertexFactoryTypeNameMap(TMap<FName, TArray<FName>>& VertexFactoryTypeNameMap) const;
	void GetWidgetFromShaderTypeNameField(FShaderTypeNameField* ShaderTypeNameField);
	void GetWidgetFromStaticSwitchParameter(TMap<FName, TPair<UObject*, UMaterialExpression*>>& StaticSwitchParametersMap);
	
	
	// Static Switch Parameter Button Clicked Event
	FReply OnStaticSwitchParameterButtonClicked(TPair<UObject*, UMaterialExpression*> InMaterialExpression);

	void AddMapToScrollBox(TMap<FString, FShaderTypeNameField*>& ShaderTypeNameChildrenField, TSharedPtr<SScrollBox>& ScrollBox, int TreeLevel = 1);

	TSharedPtr<SScrollBox> TopScrollBox;

	FShaderVariantPtr ShaderVariant = nullptr;
};
