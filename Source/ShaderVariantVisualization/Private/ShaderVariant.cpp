#include "ShaderVariant.h"

#include "MaterialEditorModule.h"
#include "Internationalization/Regex.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstance.h"

FShaderTypeNameField::FShaderTypeNameField(FString& ShaderTypeNameFieldString)
	: ShaderTypeNameFieldString(ShaderTypeNameFieldString)
{
		
}

void FShaderTypeNameField::InsertChildrenField(TArray<FString>& ChildrenField, int TypeFiledIndex)
{
	if(TypeFiledIndex >= ChildrenField.Num())
	{
		return;
	}
	FString TypeField = ChildrenField[TypeFiledIndex];

	bool bIsContain = ShaderTypeNameChildrenField.Contains(TypeField);
	
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
	

	/**************** Test Begin ***************/
	/*FMaterialRenderProxy* MaterialRenderProxy = MaterialInterface->GetRenderProxy();


	const FMaterial* CurrentMaterial = MaterialRenderProxy->GetMaterial(CurrentFeatureLevel);
	FString OutSource;

	FMaterial* MutableMaterial = const_cast<FMaterial*>(CurrentMaterial);
	FUniformExpressionSet EmptyExpressions = CurrentMaterial->GetUniformExpressions();
	bool bIsSuccess = MutableMaterial->GetMaterialExpressionSource(OutSource);*/
	/**************** Test End ***************/

	CurrentMaterial = MaterialInterface->GetMaterial();
	CurrentMaterial->GetAllReferencedExpressions(OutExpressions, nullptr);
	CurrentParentMaterial = MaterialInterface->GetBaseMaterial();
	
	MaterialInterface->SetFeatureLevelToCompile(CurrentFeatureLevel, true);
	MaterialInterface->ForceRecompileForRendering();
	const FMaterialResource* MaterialResource = MaterialInterface->GetMaterialResource(CurrentFeatureLevel); // TODO: Quality don't set!!!
	const FMaterialShaderMap* MaterialShaderMap = MaterialResource->GetGameThreadShaderMap();
	
	// bCompilationFinished = MaterialResource->IsCompilationFinished() && (MaterialShaderMap != nullptr);
	bCompilationFinished = MaterialShaderMap != nullptr;
	
	if(bCompilationFinished) // TODO: judge is compile finished
	{
		MaterialShaderMap->GetShaderList(ShaderMapList);
		ShaderTypeNameField = new FShaderTypeNameField(MaterialNameString);
	
		FStaticParameterSet StaticParameterSet;
		MaterialResource->GetStaticParameterSet(CurrentPlatform, StaticParameterSet);
		GetStaticSwitchParameters(StaticParameterSet);
		ProcessShaderVariantMessages();
	}
}

FShaderVariant::~FShaderVariant()
{
	// TODO: Ptr don't delete
}

void FShaderVariant::ProcessShaderVariantMessages()
{
	for(const auto& ShaderMapElement : ShaderMapList)
	{
		FName VertexFactoryTypeName = ShaderMapElement.Key.VFType->GetFName();
		FName ShaderTypeName = ShaderMapElement.Key.Type->GetFName();

		GetVertexFactoryType(VertexFactoryTypeName, ShaderTypeName);
		GetShaderType(ShaderTypeName);
	}
}

void FShaderVariant::GetVertexFactoryType(FName& VertexFactoryTypeName, const FName& ShaderTypeName)
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

void FShaderVariant::GetShaderType(const FName& ShaderTypeName) const 
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
	
	ShaderTypeNameField->InsertChildrenField(ShaderTypeNameFieldArray);

}

void FShaderVariant::GetStaticSwitchParameters(FStaticParameterSet& InSet)
{
	for(auto& StaticSwitchParameter : InSet.StaticSwitchParameters)
	{
		StaticSwitchParametersMap.Emplace(StaticSwitchParameter.ParameterInfo.Name, StaticSwitchParameter.Value);
	}
}
