﻿#pragma once
#include "FPlatformExternFiles.h"
#include "FPatcherSpecifyAsset.h"
#include "FPlatformExternAssets.h"
#include "Struct/AssetManager/FAssetDependenciesInfo.h"
// engine
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "HotPatcherSettingBase.generated.h"

USTRUCT(BlueprintType)
struct HOTPATCHERRUNTIME_API FPatcherEntitySettingBase
{
    GENERATED_BODY();
    virtual ~FPatcherEntitySettingBase(){}
};
USTRUCT(BlueprintType)
struct HOTPATCHERRUNTIME_API FHotPatcherSettingBase:public FPatcherEntitySettingBase
{
    GENERATED_USTRUCT_BODY()

    virtual TArray<FDirectoryPath>& GetAssetIncludeFilters();
    virtual TArray<FDirectoryPath>& GetAssetIgnoreFilters();
    virtual TArray<FPatcherSpecifyAsset>& GetIncludeSpecifyAssets();
    virtual TArray<FPlatformExternAssets>& GetAddExternAssetsToPlatform();
    virtual TMap<FString,FAssetDependenciesInfo>& GetAssetsDependenciesScanedCaches();
    virtual bool IsScanCacheOptimize()const{ return bScanCacheOptimize; }
    virtual void Init();

    virtual TArray<FExternFileInfo> GetAllExternFilesByPlatform(ETargetPlatform InTargetPlatform,bool InGeneratedHash = false);
    virtual TMap<ETargetPlatform,FPlatformExternFiles> GetAllPlatfotmExternFiles(bool InGeneratedHash = false);
    virtual TArray<FExternFileInfo> GetAddExternFilesByPlatform(ETargetPlatform InTargetPlatform);
    virtual TArray<FExternDirectoryInfo> GetAddExternDirectoryByPlatform(ETargetPlatform InTargetPlatform);

    virtual FString GetSaveAbsPath()const;
    FORCEINLINE virtual bool IsStandaloneMode()const {return bStandaloneMode;}
    FORCEINLINE virtual bool IsSaveConfig()const {return bStorageConfig;}
    FORCEINLINE virtual TArray<FString> GetAdditionalCommandletArgs()const{return AdditionalCommandletArgs;}
    FORCEINLINE virtual FString GetCombinedAdditionalCommandletArgs()const
    {
        FString Result;
        for(const auto& Option:GetAdditionalCommandletArgs())
        {
            Result+=FString::Printf(TEXT("%s "),*Option);
        }
        return Result;
    }
    
    virtual ~FHotPatcherSettingBase(){}
public:
    // backup current project Cooked/PLATFORM/PROJECTNAME/Metadata directory
    UPROPERTY(EditAnywhere, Category = "SaveTo")
    bool bStorageConfig = true;
    UPROPERTY(EditAnywhere, Category = "SaveTo")
    FDirectoryPath SavePath;
    // create a UE4Editor-cmd.exe process execute patch mission.
    UPROPERTY(EditAnywhere, Category = "Advanced")
    bool bStandaloneMode = true;
    UPROPERTY(EditAnywhere, Category = "Advanced")
    TArray<FString> AdditionalCommandletArgs;
    // UPROPERTY(EditAnywhere, Category = "Advanced")
    bool bScanCacheOptimize=true;
protected:
    TMap<FString,FAssetDependenciesInfo> ScanedCaches;
};