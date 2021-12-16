#pragma once

#include "Commandlets/Commandlet.h"
#include "HotShaderPatchCommandlet.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHotShaderPatchCommandlet, All, All);

UCLASS()
class UHotShaderPatchCommandlet :public UCommandlet
{
	GENERATED_BODY()

public:

	virtual int32 Main(const FString& Params)override;
};