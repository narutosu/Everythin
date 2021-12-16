#pragma once

#include "Commandlets/Commandlet.h"
#include "HotMultiCookerCommandlet.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHotMultiCookerCommandlet, All, All);

UCLASS()
class UHotMultiCookerCommandlet :public UCommandlet
{
	GENERATED_BODY()

public:

	virtual int32 Main(const FString& Params)override;
};