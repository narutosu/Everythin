#pragma once

#include "Containers/Map.h"
#include "AssetManager/FAssetDependenciesDetail.h"
#include "CoreMinimal.h"
#include "FAssetDependenciesInfo.generated.h"

USTRUCT(BlueprintType)
struct ASSETMANAGEREX_API FAssetDependenciesInfo
{
	GENERATED_USTRUCT_BODY()
	FAssetDependenciesInfo()=default;
	FAssetDependenciesInfo(const FAssetDependenciesInfo&)=default;
		//UPROPERTY(EditAnywhere, BlueprintReadWrite)
		//FString mAssetRef;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TMap<FString,FAssetDependenciesDetail> AssetsDependenciesMap;
};
