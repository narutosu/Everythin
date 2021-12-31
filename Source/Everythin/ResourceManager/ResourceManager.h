// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/StreamableManager.h"
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
	//无缝加载切换关卡
	UFUNCTION(BlueprintCallable, Category = "Study")
	bool SeamlessTravel(FString name);

	//切换关卡
	UFUNCTION(BlueprintCallable, Category = "Study")
	bool SyncTravel(FString name);

	UFUNCTION(BlueprintCallable, Category = "Study")
	void BeginLoadingScreen(const FString& MapName);
public:
	
	void EndLoadingScreen(UWorld* world);

	FStreamableManager& GetStreamMgr();
private:
	void OnAssetLoaded(UObject* Asset);
	static void OnAssetAdded(const FAssetData& Asset);
private:
	UGameInstance* m_GameInstance;
	FStreamableManager m_StreamableManager;
};
