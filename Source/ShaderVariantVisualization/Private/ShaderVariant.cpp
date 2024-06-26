#include "ShaderVariant.h"

#include "MaterialEditorModule.h"
#include "Internationalization/Regex.h"
#include "Materials/MaterialExpressionStaticBool.h"
#include "Materials/MaterialExpressionStaticBoolParameter.h"
#include "Materials/MaterialExpressionStaticSwitch.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstance.h"

FShaderTypeNameField::

FShaderTypeNameField(FString& ShaderTypeNameFieldString, int InVariantNum)
	: ShaderTypeNameFieldString(ShaderTypeNameFieldString), VariantNum(InVariantNum)
{
		
}

void FShaderTypeNameField::InsertChildrenField(TArray<FString>& ChildrenField, int TypeFiledIndex)
{
	if(TypeFiledIndex >= ChildrenField.Num()) 
	{
		return;
	}
	
	this->VariantNum ++;
	FString TypeField = ChildrenField[TypeFiledIndex];
	const bool bIsContain = ShaderTypeNameChildrenField.Contains(TypeField);
	if(!bIsContain)
	{
		ShaderTypeNameChildrenField.Emplace(TypeField, new FShaderTypeNameField(TypeField));
		
	}
	ShaderTypeNameChildrenField[TypeField]->InsertChildrenField(ChildrenField, TypeFiledIndex + 1);
		
}

FShaderVariant::FShaderVariant(UMaterialInterface* MaterialInterface)
{
	CurrentPlatform = GetFeatureLevelShaderPlatform(CurrentFeatureLevel); 
	
	FString MaterialNameString = MaterialInterface->GetFName().ToString();
	ShaderTypeNameField = new FShaderTypeNameField(MaterialNameString);
	
	// Process Static Switch Messages
	TArray<UMaterialExpression*> OutExpressions; 
	UMaterial* BaseMaterial = MaterialInterface->GetBaseMaterial();
	BaseMaterial->GetAllReferencedExpressions(OutExpressions, nullptr);
	GetStaticSwitchParameterFromArray(OutExpressions, BaseMaterial); 

	MaterialInterface->SetFeatureLevelToCompile(CurrentFeatureLevel, true);
	MaterialInterface->ForceRecompileForRendering();
	
	const FMaterialResource* MaterialResource = MaterialInterface->GetMaterialResource(CurrentFeatureLevel); // TODO: Quality don't set!!!
	const FMaterialShaderMap* MaterialShaderMap = MaterialResource->GetGameThreadShaderMap();
	
	// TODO: judge is compile finished
	TMap<FShaderId, TShaderRef<FShader>> ShaderMapList;
	MaterialShaderMap->GetShaderList(ShaderMapList);
	ProcessShaderVariantMessages(ShaderMapList);
	
}

FShaderVariant::~FShaderVariant()
{
	delete ShaderTypeNameField;
}

void FShaderVariant::ProcessShaderVariantMessages(const TMap<FShaderId, TShaderRef<FShader>>& InShaderMapList)
{
	for(const auto& ShaderMapElement : InShaderMapList)
	{
		FName VertexFactoryTypeName = ShaderMapElement.Key.VFType->GetFName();
		FName ShaderTypeName = ShaderMapElement.Key.Type->GetFName();

		GetVertexFactoryType(VertexFactoryTypeName, ShaderTypeName);
		GetShaderType(ShaderTypeName, VertexFactoryTypeName);
	}
}

void FShaderVariant::GetVertexFactoryType(const FName& VertexFactoryTypeName, const FName& ShaderTypeName)
{
	if(VertexFactoryTypeNameMap.Contains(VertexFactoryTypeName))
	{
		VertexFactoryTypeNameMap[VertexFactoryTypeName].Add(ShaderTypeName);
	}
	else
	{
		TArray<FName> TempArray;
		TempArray.Add(ShaderTypeName);
		
		VertexFactoryTypeNameMap.Emplace(VertexFactoryTypeName, TempArray);
	}
}

void FShaderVariant::GetShaderType(const FName& ShaderTypeName, const FName& VertexFactoryTypeName) const
{
	const FName ToMatchName = ShaderTypeName;
	const FString ToMatchString = ToMatchName.ToString();
	
	TArray<FString> ShaderTypeNameFieldArray;
	FString PatternString;
	
	switch (CurrentFeatureLevel)
	{
		case ERHIFeatureLevel::SM5:
			PatternString = FString("(TBasePass[PVDH]S)(.+Policy)(.*)?");
			break;
		case ERHIFeatureLevel::ES3_1:
			PatternString = FString("(TMobileBasePass[PVDH]S)(.+Policy)(.*)?");
			break;
		default:
			PatternString = FString("(TBasePass[PVDH]S)(.+Policy)(.*)?");
			break;
	}
	const FRegexPattern ShaderTypePattern(PatternString);
	FRegexMatcher ShaderTypeMatcher(ShaderTypePattern, ToMatchString);
	
	if (ShaderTypeMatcher.FindNext())
	{
		const FString MatchedPart1 = ShaderTypeMatcher.GetCaptureGroup(1);
		const FString MatchedPart2 = ShaderTypeMatcher.GetCaptureGroup(2);
		const FString MatchedPart3 = ShaderTypeMatcher.GetCaptureGroup(3);
		ShaderTypeNameFieldArray.Add(MatchedPart1);
		ShaderTypeNameFieldArray.Add(MatchedPart2);
		ShaderTypeNameFieldArray.Add(MatchedPart3);
	}

	ShaderTypeNameFieldArray.Add(VertexFactoryTypeName.ToString());
	
	ShaderTypeNameField->InsertChildrenField(ShaderTypeNameFieldArray);
}

void FShaderVariant::GetStaticSwitchParameterFromArray(const TArray<UMaterialExpression*>& InExpressions, UObject* Asset)
{
	for(auto& Expression : InExpressions)
	{
		GetStaticSwitchParameterFromExpression(Expression, Asset);
	}
}

void FShaderVariant::GetStaticSwitchParameterFromExpression(UMaterialExpression* InExpressions, UObject* Asset)
{
	if(InExpressions->IsA<UMaterialExpressionStaticSwitch>())
	{
		const UMaterialExpressionStaticSwitch* StaticSwitch = CastChecked<UMaterialExpressionStaticSwitch>(InExpressions);
		if(StaticSwitch->Value.Expression->IsA<UMaterialExpressionStaticBoolParameter>())
		{
			StaticSwitchParametersMap.Add(StaticSwitch->Value.Expression->GetParameterName(), TPair<UObject*, UMaterialExpression* >{Asset ,StaticSwitch->Value.Expression});
		}
		else if(StaticSwitch->Value.Expression->IsA<UMaterialExpressionStaticBool>())
		{
			// use Desc as Parameter name
			StaticSwitchParametersMap.Add(FName(*(StaticSwitch->Value.Expression->Desc)), TPair<UObject*, UMaterialExpression* >{Asset ,StaticSwitch->Value.Expression});
		}
		else
		{
			checkf(false, TEXT("Unexpect Value for Static Switch Expression!"));
		}
	}
	else if(InExpressions->IsA<UMaterialExpressionMaterialFunctionCall>())
	{
		const UMaterialExpressionMaterialFunctionCall* MaterialFunctionCall = CastChecked<UMaterialExpressionMaterialFunctionCall>(InExpressions);
		UMaterialFunctionInterface* MaterialFunction = MaterialFunctionCall->MaterialFunction;
		const TArray<UMaterialExpression*>& FunctionExpressions = *MaterialFunction->GetFunctionExpressions();
		GetStaticSwitchParameterFromArray(FunctionExpressions, MaterialFunction);
	}
	else if(InExpressions->IsA<UMaterialExpressionMaterialAttributeLayers>())
	{
		const UMaterialExpressionMaterialAttributeLayers* MaterialAttributeLayers = CastChecked<UMaterialExpressionMaterialAttributeLayers>(InExpressions);
		FMaterialLayersFunctions LayersFunctions = MaterialAttributeLayers->DefaultLayers;
		for(const auto& Layer : LayersFunctions.Layers)
		{
			const TArray<UMaterialExpression*>& FunctionExpressions = *Layer->GetFunctionExpressions();
			GetStaticSwitchParameterFromArray(FunctionExpressions);
		}
		for(const auto& Blend : LayersFunctions.Blends)
		{
			const TArray<UMaterialExpression*>& FunctionExpressions = *Blend->GetFunctionExpressions();
			GetStaticSwitchParameterFromArray(FunctionExpressions);
		}
	}	
	
}

