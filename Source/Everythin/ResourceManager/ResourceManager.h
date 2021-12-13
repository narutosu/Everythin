// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ResourceManager.generated.h"
DECLARE_LOG_CATEGORY_EXTERN(ResourceManagerLog, All, All);
class UGameInstance;
/**
 * 
 */
UCLASS()
class EVERYTHIN_API UResourceManager : public UObject
{
	GENERATED_BODY()
	UResourceManager();
	~UResourceManager();
public:
	//无缝加载关卡
	UFUNCTION(BlueprintCallable, Category = "Study")
	bool SeamlessTravel(FString name);
private:
	UGameInstance* m_GameInstance;
};
