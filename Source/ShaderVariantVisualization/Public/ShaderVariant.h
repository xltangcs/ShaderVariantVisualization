#pragma once


struct FShaderTypeNameField
{
	FString ShaderTypeNameFieldString;
	TMap<FString, FShaderTypeNameField*> ShaderTypeNameChildrenField;

	FShaderTypeNameField(FString& ShaderTypeNameFieldString);

	void InsertChildrenField(TArray<FString>& ChildrenField, int TypeFiledIndex = 0);
};



class FShaderVariant
{
public:
	FShaderVariant(class UMaterialInterface* MaterialInterface);
	~FShaderVariant();

	
	TMap<FName, TArray<FName>>& GetVertexFactoryTypeNameMap() { return VertexFactoryTypeNameMap; }
	FShaderTypeNameField* GetShaderTypeNameField() const { return ShaderTypeNameField; }
	TMap<FName, bool>& GetStaticSwitchParametersMap() { return  StaticSwitchParametersMap; }

	
	TArray<UMaterialExpression*> OutExpressions; //TODO: Private
	
private:
	void ProcessShaderVariantMessages();
	
	void GetVertexFactoryType(FName& VertexFactoryTypeName, const FName& ShaderTypeName);
	void GetShaderType(const FName& ShaderTypeName) const;
	void GetStaticSwitchParameters(FStaticParameterSet& InSet);


	ERHIFeatureLevel::Type CurrentFeatureLevel = ERHIFeatureLevel::Type::ES3_1;
	EMaterialQualityLevel::Type CurrentQualityLevel = EMaterialQualityLevel::Low;
	EShaderPlatform CurrentPlatform;
	
	TMap<FShaderId, TShaderRef<FShader>> ShaderMapList;
	
	TMap<FName, TArray<FName>> VertexFactoryTypeNameMap;
	FShaderTypeNameField* ShaderTypeNameField;
	TMap<FName, bool> StaticSwitchParametersMap;

	// is compile finished
	bool bCompilationFinished = false;

public:
	UMaterial* CurrentMaterial = nullptr;
	UMaterial* CurrentParentMaterial = nullptr;
	
};


