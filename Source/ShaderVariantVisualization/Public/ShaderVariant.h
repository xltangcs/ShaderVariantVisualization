#pragma once


struct FShaderTypeNameField
{
	FString ShaderTypeNameFieldString = FString("Please Set Name");
	TMap<FString, FShaderTypeNameField*> ShaderTypeNameChildrenField;
	int VariantNum = 0;

	FShaderTypeNameField(FString& ShaderTypeNameFieldString, int InVariantNum = 0);

	void InsertChildrenField(TArray<FString>& ChildrenField, int TypeFiledIndex = 0);
};



class FShaderVariant
{
public:
	FShaderVariant(class UMaterialInterface* MaterialInterface);
	~FShaderVariant();

	
	TMap<FName, TArray<FName>>& GetVertexFactoryTypeNameMap() { return VertexFactoryTypeNameMap; }
	FShaderTypeNameField* GetShaderTypeNameField() const { return ShaderTypeNameField; }
	TMap<FName, TPair<UObject*, UMaterialExpression*>>& GetStaticSwitchParametersMap() { return  StaticSwitchParametersMap; }

private:
	void ProcessShaderVariantMessages(const TMap<FShaderId, TShaderRef<FShader>>& InShaderMapList);
	
	void GetVertexFactoryType(const FName& VertexFactoryTypeName, const FName& ShaderTypeName);
	void GetShaderType(const FName& ShaderTypeName, const FName& VertexFactoryTypeName) const ;
	void GetStaticSwitchParameterFromArray(const TArray<UMaterialExpression*>& InExpressions, UObject* Asset = nullptr);
	void GetStaticSwitchParameterFromExpression(UMaterialExpression* InExpressions, UObject* Asset = nullptr);
	
	ERHIFeatureLevel::Type CurrentFeatureLevel = ERHIFeatureLevel::Type::ES3_1;
	EMaterialQualityLevel::Type CurrentQualityLevel = EMaterialQualityLevel::Low;
	EShaderPlatform CurrentPlatform;
	
	TMap<FName, TArray<FName>> VertexFactoryTypeNameMap;
	FShaderTypeNameField* ShaderTypeNameField = nullptr;
	TMap<FName, TPair<UObject*, UMaterialExpression*>> StaticSwitchParametersMap;
};


