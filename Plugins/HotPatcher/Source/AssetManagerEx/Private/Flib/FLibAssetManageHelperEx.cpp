// Fill out your copyright notice in the Description page of Project Settings.


#include "FLibAssetManageHelperEx.h"
#include "AssetManager/FAssetDependenciesInfo.h"
#include "AssetManager/FAssetDependenciesDetail.h"
#include "AssetManager/FFileArrayDirectoryVisitor.hpp"
#include "AssetManagerExLog.h"

#include "AssetRegistryModule.h"
#include "ARFilter.h"
#include "Kismet/KismetStringLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Json.h"
#include "Templates/SharedPointer.h"
#include "Interfaces/IPluginManager.h"
#include "Engine/AssetManager.h"
#include "AssetData.h"
#include "Resources/Version.h"
#ifdef __DEVELOPER_MODE__
#include "Interfaces/ITargetPlatform.h"
#include "Interfaces/ITargetPlatformManagerModule.h"
#endif

bool GScanCacheOptimize = true;

PRAGMA_DISABLE_DEPRECATION_WARNINGS
FString UFLibAssetManageHelperEx::ConvVirtualToAbsPath(const FString& InPackagePath)
{
	FString ResultAbsPath;

	FString AssetAbsNotPostfix = FPaths::ConvertRelativePathToFull(FPackageName::LongPackageNameToFilename(UFLibAssetManageHelperEx::GetLongPackageNameFromPackagePath(InPackagePath)));
	FString AssetName = UFLibAssetManageHelperEx::GetAssetNameFromPackagePath(InPackagePath);
	FString SearchDir;
	{
		int32 FoundIndex;
		AssetAbsNotPostfix.FindLastChar('/', FoundIndex);
		if (FoundIndex != INDEX_NONE)
		{
			SearchDir = UKismetStringLibrary::GetSubstring(AssetAbsNotPostfix, 0, FoundIndex);
		}
	}

	TArray<FString> localFindFiles;
	IFileManager::Get().FindFiles(localFindFiles, *SearchDir, nullptr);

	for (const auto& Item : localFindFiles)
	{
		if (Item.Contains(AssetName) && Item[AssetName.Len()] == '.')
		{
			ResultAbsPath = FPaths::Combine(SearchDir, Item);
			break;
		}
	}

	return ResultAbsPath;
}


bool UFLibAssetManageHelperEx::ConvAbsToVirtualPath(const FString& InAbsPath, FString& OutPackagePath)
{
	bool runState = false;
	FString LongPackageName;
	runState = FPackageName::TryConvertFilenameToLongPackageName(InAbsPath, LongPackageName);

	if (runState)
	{
		FString PackagePath;
		if (UFLibAssetManageHelperEx::ConvLongPackageNameToPackagePath(LongPackageName, PackagePath))
		{
			OutPackagePath = PackagePath;
			runState = runState && true;
		}
	}
	
	return runState;
}

void UFLibAssetManageHelperEx::UpdateAssetMangerDatabase(bool bForceRefresh)
{
#if WITH_EDITOR
	UAssetManager& AssetManager = UAssetManager::Get();
	AssetManager.UpdateManagementDatabase(bForceRefresh);
#endif
}

FString UFLibAssetManageHelperEx::GetLongPackageNameFromPackagePath(const FString& InPackagePath)
{
	int32 FoundIndex;
	if (InPackagePath.FindLastChar('.', FoundIndex))
	{
		FStringAssetReference InAssetRef = InPackagePath;
		return InAssetRef.GetLongPackageName();
	}
	else
	{
		return InPackagePath;
	}
}

FString UFLibAssetManageHelperEx::GetAssetNameFromPackagePath(const FString& InPackagePath)
{
	FStringAssetReference InAssetRef = InPackagePath;
	return InAssetRef.GetAssetName();
}

FString UFLibAssetManageHelperEx::LongPackageNameToPackagePath(const FString& InLongPackageName)
{
	FString OutPackagePath;
	UFLibAssetManageHelperEx::ConvLongPackageNameToPackagePath(InLongPackageName,OutPackagePath);
	return OutPackagePath;
}


bool UFLibAssetManageHelperEx::ConvLongPackageNameToPackagePath(const FString& InLongPackageName, FString& OutPackagePath)
{
	OutPackagePath.Empty();
	bool runState = false;
	if(InLongPackageName.IsEmpty())
		return runState;
	OutPackagePath = InLongPackageName;
	
	if(InLongPackageName.Contains(TEXT(".")))
	{
		OutPackagePath = InLongPackageName;
		runState = true;
		return runState;
	}
	
	if (FPackageName::DoesPackageExist(InLongPackageName))
	{
		FString AssetName;
		{
			int32 FoundIndex;
			InLongPackageName.FindLastChar('/', FoundIndex);
			if (FoundIndex != INDEX_NONE)
			{
				AssetName = UKismetStringLibrary::GetSubstring(InLongPackageName, FoundIndex + 1, InLongPackageName.Len() - FoundIndex);
			}
		}
		OutPackagePath = InLongPackageName + TEXT(".") + AssetName;
		runState = true;
	}
	return runState;
}

bool UFLibAssetManageHelperEx::ConvPackagePathToLongPackageName(const FString& InPackagePath, FString& OutLongPackageName)
{
	bool brunstatus=false;
	FSoftObjectPath PackageSoftRef{ InPackagePath };
	if (PackageSoftRef.IsValid())
	{
		OutLongPackageName = PackageSoftRef.GetLongPackageName();
		brunstatus = true;
	}

	return brunstatus;
}

bool UFLibAssetManageHelperEx::GetAssetPackageGUID(const FString& InPackagePath, FString& OutGUID)
{
	bool bResult = false;
	if (InPackagePath.IsEmpty())
		return false;

	const FAssetPackageData* AssetPackageData = UFLibAssetManageHelperEx::GetPackageDataByPackagePath(InPackagePath);
	if (AssetPackageData != NULL)
	{
		const FGuid& AssetGuid = AssetPackageData->PackageGuid;
		OutGUID = AssetGuid.ToString();
		bResult = true;
	}
	return bResult;
}


FAssetDependenciesInfo UFLibAssetManageHelperEx::CombineAssetDependencies(const FAssetDependenciesInfo& A, const FAssetDependenciesInfo& B)
{
	FAssetDependenciesInfo resault;

	auto CombineLambda = [&resault](const FAssetDependenciesInfo& InDependencies)
	{
		TArray<FString> Keys;
		InDependencies.AssetsDependenciesMap.GetKeys(Keys);
		for (const auto& Key : Keys)
		{
			if (!resault.AssetsDependenciesMap.Contains(Key))
			{
				resault.AssetsDependenciesMap.Add(Key, *InDependencies.AssetsDependenciesMap.Find(Key));
			}
			else
			{

				{
					TMap<FString,FAssetDetail>& ExistingAssetDetails = resault.AssetsDependenciesMap.Find(Key)->AssetDependencyDetails;
					TArray<FString> ExistingAssetList;
					ExistingAssetDetails.GetKeys(ExistingAssetList);

					const TMap<FString,FAssetDetail>& PaddingAssetDetails = InDependencies.AssetsDependenciesMap.Find(Key)->AssetDependencyDetails;
					TArray<FString> PaddingAssetList;
					PaddingAssetDetails.GetKeys(PaddingAssetList);

					for (const auto& PaddingDetailItem : PaddingAssetList)
					{
						if (!ExistingAssetDetails.Contains(PaddingDetailItem))
						{
							ExistingAssetDetails.Add(PaddingDetailItem,*PaddingAssetDetails.Find(PaddingDetailItem));
						}
					}
				}
			}
		}
	};

	CombineLambda(A);
	CombineLambda(B);

	return resault;
}

void UFLibAssetManageHelperEx::GetAssetDependencies(
	const FString& InLongPackageName,
	const TArray<EAssetRegistryDependencyTypeEx>& AssetRegistryDependencyTypes,
	FAssetDependenciesInfo& OutDependices,
	TMap<FString, FAssetDependenciesInfo>& ScanedCaches
)
{
	if (InLongPackageName.IsEmpty())
		return;

	FStringAssetReference AssetRef = FStringAssetReference(InLongPackageName);
	if (!AssetRef.IsValid())
		return;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	
	if (FPackageName::DoesPackageExist(InLongPackageName))
	{
		{
			TArray<FAssetData> AssetDataList;
			bool bResault = AssetRegistryModule.Get().GetAssetsByPackageName(FName(*InLongPackageName), AssetDataList);
			if (!bResault || !AssetDataList.Num())
			{
				UE_LOG(LogAssetManagerEx, Error, TEXT("Faild to Parser AssetData of %s, please check."), *InLongPackageName);
				return;
			}
			if (AssetDataList.Num() > 1)
			{
				for(const auto& Asset:AssetDataList)
				{
					UE_LOG(LogAssetManagerEx,Log,TEXT("AssetData ObjectPath %s"),*Asset.ObjectPath.ToString())
					UE_LOG(LogAssetManagerEx,Log,TEXT("AssetData AssetName %s"),*Asset.AssetName.ToString())
				}
				UE_LOG(LogAssetManagerEx, Warning, TEXT("Got mulitple AssetData of %s,please check."), *InLongPackageName);
			}
		}
		UFLibAssetManageHelperEx::GatherAssetDependicesInfoRecursively(AssetRegistryModule, InLongPackageName, AssetRegistryDependencyTypes, OutDependices,ScanedCaches);
	}
}

void UFLibAssetManageHelperEx::GetAssetListDependencies(
	const TArray<FString>& InLongPackageNameList,
	const TArray<EAssetRegistryDependencyTypeEx>& AssetRegistryDependencyTypes,
	FAssetDependenciesInfo& OutDependices,
	TMap<FString, FAssetDependenciesInfo>& ScanedCaches
)
{
	FAssetDependenciesInfo result;

	for (const auto& LongPackageItem : InLongPackageNameList)
	{
		FAssetDependenciesInfo CurrentDependency;
		UFLibAssetManageHelperEx::GetAssetDependencies(LongPackageItem, AssetRegistryDependencyTypes, CurrentDependency,ScanedCaches);
		result = UFLibAssetManageHelperEx::CombineAssetDependencies(result, CurrentDependency);
	}
	OutDependices = result;
}


bool UFLibAssetManageHelperEx::GetAssetDependency(
	const FString& InLongPackageName,
	const TArray<EAssetRegistryDependencyTypeEx>& AssetRegistryDependencyTypes,
	TArray<FAssetDetail>& OutRefAsset,
	TMap<FString, FAssetDependenciesInfo>& ScanedCaches,
	bool bRecursively
)
{
	bool bStatus = false;
	if (FPackageName::DoesPackageExist(InLongPackageName))
	{
		OutRefAsset.Empty();
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

		FAssetDependenciesInfo AssetDep;
		UFLibAssetManageHelperEx::GatherAssetDependicesInfoRecursively(AssetRegistryModule, InLongPackageName, AssetRegistryDependencyTypes, AssetDep,ScanedCaches,bRecursively);
		UFLibAssetManageHelperEx::GetAssetDetailsByAssetDependenciesInfo(AssetDep, OutRefAsset);
		bStatus = true;
	}
	return bStatus;
}

bool UFLibAssetManageHelperEx::GetAssetDependencyByDetail(
	const FAssetDetail& InAsset,
	const TArray<EAssetRegistryDependencyTypeEx>& AssetRegistryDependencyTypes,
	TArray<FAssetDetail>& OutRefAsset,
	TMap<FString, FAssetDependenciesInfo>& ScanedCaches,
	bool bRecursively /*= true*/)
{
	FString LongPackageName;
	UFLibAssetManageHelperEx::ConvPackagePathToLongPackageName(InAsset.mPackagePath, LongPackageName);
	
	return UFLibAssetManageHelperEx::GetAssetDependency(LongPackageName, AssetRegistryDependencyTypes, OutRefAsset,ScanedCaches, bRecursively);
}

bool UFLibAssetManageHelperEx::GetAssetReference(const FAssetDetail& InAsset, const TArray<EAssetRegistryDependencyType::Type>& SearchAssetDepTypes, TArray<FAssetDetail>& OutRefAsset)
{
	bool bStatus = false;
	FString LongPackageName;
	if (UFLibAssetManageHelperEx::ConvPackagePathToLongPackageName(InAsset.mPackagePath, LongPackageName))
	{
		TArray<FAssetIdentifier> AssetIdentifier;

		FAssetIdentifier InAssetIdentifier;
		InAssetIdentifier.PackageName = FName(*LongPackageName);
		AssetIdentifier.Add(InAssetIdentifier);

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
		TArray<FAssetIdentifier> ReferenceNames;
		for (const FAssetIdentifier& AssetId : AssetIdentifier)
		{
			for (const auto& AssetDepType : SearchAssetDepTypes)
			{
				TArray<FAssetIdentifier> CurrentTypeReferenceNames;
#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION >=26
				AssetRegistryModule.Get().GetReferencers(AssetId, CurrentTypeReferenceNames, (UE::AssetRegistry::EDependencyCategory)((uint8)AssetDepType));
#else
				AssetRegistryModule.Get().GetReferencers(AssetId, CurrentTypeReferenceNames, AssetDepType);
#endif
				for (const auto& Name : CurrentTypeReferenceNames)
				{
					if (!(Name.PackageName.ToString() == LongPackageName))
					{
						ReferenceNames.AddUnique(Name);
					}
				}
			}
			
		}

		for (const auto& RefAssetId : ReferenceNames)
		{
			FAssetDetail RefAssetDetail;
			if (UFLibAssetManageHelperEx::GetSpecifyAssetDetail(RefAssetId.PackageName.ToString(), RefAssetDetail))
			{
				OutRefAsset.Add(RefAssetDetail);
			}
		}
		bStatus = true;
	}
	return bStatus;
}

void UFLibAssetManageHelperEx::GetAssetReferenceRecursively(const FAssetDetail& InAsset,
                                                            const TArray<EAssetRegistryDependencyType::Type>&
                                                            SearchAssetDepTypes,
                                                            const TArray<FString>& SearchAssetsTypes,
                                                            TArray<FAssetDetail>& OutRefAsset)
{
	TArray<FAssetDetail> CurrentAssetsRef;
	UFLibAssetManageHelperEx::GetAssetReference(InAsset,SearchAssetDepTypes,CurrentAssetsRef);

	auto MatchAssetsTypesLambda = [](const FAssetDetail& InAsset,const TArray<FString>& SearchAssetsTypes)->bool
	{
		bool bresult = false;
		if(SearchAssetsTypes.Num() > 0)
		{
			for(const auto& AssetType:SearchAssetsTypes)
			{
				if(InAsset.mAssetType.Equals(AssetType))
				{
					bresult = true;
					break;
				}
			}
		}
		else
		{
			bresult = true;
		}
		return bresult;
	};
	
    for(const auto& AssetRef:CurrentAssetsRef)
    {
    	if(MatchAssetsTypesLambda(AssetRef,SearchAssetsTypes))
    	{
    		if(!OutRefAsset.Contains(AssetRef))
    		{
    			OutRefAsset.AddUnique(AssetRef);
    			UFLibAssetManageHelperEx::GetAssetReferenceRecursively(AssetRef,SearchAssetDepTypes,SearchAssetsTypes,OutRefAsset);
    		}
    	}
    }
}

bool UFLibAssetManageHelperEx::GetAssetReferenceEx(const FAssetDetail& InAsset, const TArray<EAssetRegistryDependencyTypeEx>& SearchAssetDepTypes, TArray<FAssetDetail>& OutRefAsset)
{
	TArray<EAssetRegistryDependencyType::Type> local_SearchAssetDepTypes;
	for (const auto& type : SearchAssetDepTypes)
	{
		local_SearchAssetDepTypes.AddUnique(UFLibAssetManageHelperEx::ConvAssetRegistryDependencyToInternal(type));
	}

	return UFLibAssetManageHelperEx::GetAssetReference(InAsset, local_SearchAssetDepTypes, OutRefAsset);
}

void UFLibAssetManageHelperEx::GetAssetDependenciesForAssetDetail(
	const FAssetDetail& InAssetDetail,
	const TArray<EAssetRegistryDependencyTypeEx>& AssetRegistryDependencyTypes,
	FAssetDependenciesInfo& OutDependices,
	TMap<FString, FAssetDependenciesInfo>& ScandCaches
)
{
	FString AssetPackagePath = InAssetDetail.mPackagePath;
	FSoftObjectPath AssetObjectSoftRef{ AssetPackagePath };

	UFLibAssetManageHelperEx::GetAssetDependencies(AssetObjectSoftRef.GetLongPackageName(), AssetRegistryDependencyTypes, OutDependices,ScandCaches);
}

void UFLibAssetManageHelperEx::GetAssetListDependenciesForAssetDetail(
			const TArray<FAssetDetail>& InAssetsDetailList, 
			const TArray<EAssetRegistryDependencyTypeEx>& AssetRegistryDependencyTypes,
			FAssetDependenciesInfo& OutDependices,
			TMap<FString, FAssetDependenciesInfo>& ScandCaches
)
{
	FAssetDependenciesInfo result;
	for (const auto& AssetDetail : InAssetsDetailList)
	{
		FString AssetPackageName;
		UFLibAssetManageHelperEx::ConvPackagePathToLongPackageName(AssetDetail.mPackagePath,AssetPackageName);
		FAssetDependenciesInfo CurrentAsserInfo;

		if(GScanCacheOptimize && ScandCaches.Find(AssetPackageName))
		{
			CurrentAsserInfo = *ScandCaches.Find(AssetPackageName);
		}
		else
		{
			UFLibAssetManageHelperEx::GetAssetDependenciesForAssetDetail(AssetDetail, AssetRegistryDependencyTypes,CurrentAsserInfo,ScandCaches);
			if(GScanCacheOptimize && !ScandCaches.Find(AssetPackageName))
			{
				ScandCaches.Add(AssetPackageName,CurrentAsserInfo);
			}
		}
		
		result = UFLibAssetManageHelperEx::CombineAssetDependencies(result, CurrentAsserInfo);
	}
	OutDependices = result;
}

void UFLibAssetManageHelperEx::GetAssetDetailsByAssetDependenciesInfo(const FAssetDependenciesInfo& InAssetDependencies,TArray<FAssetDetail>& OutAssetDetails)
{
		OutAssetDetails.Empty();
		TArray<FString> Keys;
		InAssetDependencies.AssetsDependenciesMap.GetKeys(Keys);

		for (const auto& Key : Keys)
		{
			TMap<FString, FAssetDetail> ModuleAssetDetails = InAssetDependencies.AssetsDependenciesMap.Find(Key)->AssetDependencyDetails;

			TArray<FString> ModuleAssetKeys;
			ModuleAssetDetails.GetKeys(ModuleAssetKeys);

			for (const auto& ModuleAssetKey : ModuleAssetKeys)
			{
				OutAssetDetails.Add(*ModuleAssetDetails.Find(ModuleAssetKey));
			}
		}
}

FAssetDetail UFLibAssetManageHelperEx::GetAssetDetailByPackageName(const FString& InPackageName)
{
	FAssetDetail AssetDetail;
	UAssetManager& AssetManager = UAssetManager::Get();
	if (AssetManager.IsValid())
	{
		FString PackagePath;
		if (UFLibAssetManageHelperEx::ConvLongPackageNameToPackagePath(InPackageName, PackagePath))
		{
			FAssetData OutAssetData;
			if (AssetManager.GetAssetDataForPath(FSoftObjectPath{ PackagePath }, OutAssetData) && OutAssetData.IsValid())
			{
				AssetDetail.mPackagePath = OutAssetData.ObjectPath.ToString();
				AssetDetail.mAssetType = OutAssetData.AssetClass.ToString();
				UFLibAssetManageHelperEx::GetAssetPackageGUID(PackagePath, AssetDetail.mGuid);
			}
		}
	}
	return AssetDetail;
}

bool UFLibAssetManageHelperEx::HasAssetInDependenciesInfo(const FAssetDependenciesInfo& AssetDependencies,
	const FString& InAssetPackageName)
{
	bool bHas = false;
	FString BelongModuleName = UFLibAssetManageHelperEx::GetAssetBelongModuleName(InAssetPackageName);
	if (AssetDependencies.AssetsDependenciesMap.Contains(BelongModuleName))
	{
		bHas = AssetDependencies.AssetsDependenciesMap.Find(BelongModuleName)->AssetDependencyDetails.Contains(InAssetPackageName);
	}
	return bHas;
}

TArray<FString> UFLibAssetManageHelperEx::GetAssetLongPackageNameByAssetDependenciesInfo(const FAssetDependenciesInfo& InAssetDependencies)
{
	TArray<FString> OutAssetLongPackageName;
	TArray<FAssetDetail> OutAssetDetails;
	UFLibAssetManageHelperEx::GetAssetDetailsByAssetDependenciesInfo(InAssetDependencies, OutAssetDetails);

	for (const auto& Asset : OutAssetDetails)
	{
		FString LongPackageName;
		if (UFLibAssetManageHelperEx::ConvPackagePathToLongPackageName(Asset.mPackagePath, LongPackageName))
		{
			OutAssetLongPackageName.AddUnique(LongPackageName);
		}
	}
	return OutAssetLongPackageName;
}

void UFLibAssetManageHelperEx::GatherAssetDependicesInfoRecursively(
	FAssetRegistryModule& InAssetRegistryModule,
	const FString& InTargetLongPackageName,
	const TArray<EAssetRegistryDependencyTypeEx>& InAssetDependencyTypes,
	FAssetDependenciesInfo& OutDependencies,
	TMap<FString, FAssetDependenciesInfo>& ScanedCaches,
	bool bRecursively,
	TSet<FString> IgnoreAssetsPackageNames
)
{
	IgnoreAssetsPackageNames.Add(InTargetLongPackageName);
	TArray<FName> local_Dependencies;
	// TArray<EAssetRegistryDependencyType::Type> AssetTypes;
	
	bool bGetDependenciesSuccess = false;
	EAssetRegistryDependencyType::Type TotalType = EAssetRegistryDependencyType::None;

	for (const auto& DepType : InAssetDependencyTypes)
	{
		TotalType = (EAssetRegistryDependencyType::Type)((uint8)TotalType | (uint8)UFLibAssetManageHelperEx::ConvAssetRegistryDependencyToInternal(DepType));
	}

	// AssetTypes.AddUnique(UFLibAssetManageHelperEx::ConvAssetRegistryDependencyToInternal(DepType));
#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION >=26
	bGetDependenciesSuccess = InAssetRegistryModule.Get().GetDependencies(FName(*InTargetLongPackageName), local_Dependencies, (UE::AssetRegistry::EDependencyCategory)((uint8)TotalType));
#else
	bGetDependenciesSuccess = InAssetRegistryModule.Get().GetDependencies(FName(*InTargetLongPackageName), local_Dependencies, TotalType);
#endif
	if (!bGetDependenciesSuccess)
		return;

	TMap<FString,FAssetDetail> AssetDependenciesDetailMap;
		
	for (auto &DependItem : local_Dependencies)
	{
		FString LongDependentPackageName = DependItem.ToString();

		// ignore /Script/WeatherSystem and self
		if(LongDependentPackageName.StartsWith(TEXT("/Script/")) || LongDependentPackageName.Equals(InTargetLongPackageName))
			continue;

		FAssetDetail AssetDetail = UFLibAssetManageHelperEx::GetAssetDetailByPackageName(LongDependentPackageName);
		if(AssetDetail.IsValid())
		{
			AssetDependenciesDetailMap.Add(LongDependentPackageName,AssetDetail);
		}
	}

	for(const auto& AssetDetail:AssetDependenciesDetailMap)
	{
		if(AssetDetail.Value.mPackagePath.IsEmpty())
			continue;
		FString BelongModuleName = UFLibAssetManageHelperEx::GetAssetBelongModuleName(AssetDetail.Key);
		FAssetDependenciesDetail* ModuleCategory;
		if (OutDependencies.AssetsDependenciesMap.Contains(BelongModuleName))
		{
			// UE_LOG(LogAssetManagerEx, Log, TEXT("Belong Module is %s,Asset is %s"), *BelongModuleName, *LongDependentPackageName);
			ModuleCategory = OutDependencies.AssetsDependenciesMap.Find(BelongModuleName);
		}
		else
		{
			// UE_LOG(LogAssetManagerEx, Log, TEXT("New Belong Module is %s,Asset is %s"), *BelongModuleName,*LongDependentPackageName);
			FAssetDependenciesDetail& NewModuleCategory = OutDependencies.AssetsDependenciesMap.Add(BelongModuleName, FAssetDependenciesDetail{});
			NewModuleCategory.ModuleCategory = BelongModuleName;
			ModuleCategory = OutDependencies.AssetsDependenciesMap.Find(BelongModuleName);
		}
		
		ModuleCategory->AssetDependencyDetails.Add(AssetDetail.Key, AssetDetail.Value);
		
		if (bRecursively && !IgnoreAssetsPackageNames.Contains(AssetDetail.Key))
		{
			if(!GScanCacheOptimize)
			{
				UFLibAssetManageHelperEx::GatherAssetDependicesInfoRecursively(InAssetRegistryModule, AssetDetail.Key, InAssetDependencyTypes,OutDependencies, ScanedCaches,true,IgnoreAssetsPackageNames);
			}
			else
			{
				FAssetDependenciesInfo CurrentDependencies;
				// search cached
				if(!ScanedCaches.Find(AssetDetail.Key))
				{
					UFLibAssetManageHelperEx::GatherAssetDependicesInfoRecursively(InAssetRegistryModule, AssetDetail.Key, InAssetDependencyTypes,CurrentDependencies, ScanedCaches,true,IgnoreAssetsPackageNames);
					// Add to ScanedCaches
					ScanedCaches.Add(AssetDetail.Key,CurrentDependencies);
				}
				else
				{
					CurrentDependencies = *ScanedCaches.Find(AssetDetail.Key);
				}
				
				OutDependencies = UFLibAssetManageHelperEx::CombineAssetDependencies(OutDependencies,CurrentDependencies);
			}
		}
	}
}


bool UFLibAssetManageHelperEx::GetModuleAssetsList(
	const FString& InModuleName,
	const TArray<FString>& InExFilterPackagePaths,
	const TArray<EAssetRegistryDependencyTypeEx>& AssetRegistryDependencyTypes,
	TArray<FAssetDetail>& OutAssetList,
	TMap<FString, FAssetDependenciesInfo>& ScanedCaches
)
{
	TMap<FString, FString> AllEnableModules;
	TArray<FString> AllEnableModuleNames;
	UFLibAssetManageHelperEx::GetAllEnabledModuleName(AllEnableModules);

	AllEnableModules.GetKeys(AllEnableModuleNames);

	if (!AllEnableModuleNames.Contains(InModuleName))
		return false;
	TArray<FString> AllFilterPackageNames;
	AllFilterPackageNames.AddUnique(TEXT("/") + InModuleName);
	for (const auto& ExFilterPackageName : InExFilterPackagePaths)
	{
		AllFilterPackageNames.AddUnique(ExFilterPackageName);
	}
	return UFLibAssetManageHelperEx::GetAssetsList(AllFilterPackageNames, AssetRegistryDependencyTypes, OutAssetList,ScanedCaches);
}

bool UFLibAssetManageHelperEx::GetAssetsList(
	const TArray<FString>& InFilterPackagePaths,
	const TArray<EAssetRegistryDependencyTypeEx>& AssetRegistryDependencyTypes,
	TArray<FAssetDetail>& OutAssetList,
	TMap<FString, FAssetDependenciesInfo>& ScanedCaches,
	bool bReTargetRedirector
)
{
	TArray<FAssetData> AllAssetData;
	if (UFLibAssetManageHelperEx::GetAssetsData(InFilterPackagePaths, AllAssetData,true))
	{
		for (const auto& AssetDataIndex : AllAssetData)
		{
			if (!AssetDataIndex.IsValid())
				continue;
			FAssetDetail AssetDetail;
			if (bReTargetRedirector && AssetDataIndex.IsRedirector())
			{
				TArray<FAssetDetail> TargetAssets;
				FString RedirectorLongPackageName;
				if(UFLibAssetManageHelperEx::ConvLongPackageNameToPackagePath(AssetDataIndex.PackageName.ToString(), RedirectorLongPackageName))
				{
					if(!RedirectorLongPackageName.IsEmpty())
					{
						UFLibAssetManageHelperEx::GetAssetDependency(RedirectorLongPackageName, AssetRegistryDependencyTypes, TargetAssets, ScanedCaches,false);
						if (!!TargetAssets.Num())
						{
							AssetDetail = TargetAssets[0];
						}
					}
				}
			}
			else
			{
				UFLibAssetManageHelperEx::ConvFAssetDataToFAssetDetail(AssetDataIndex, AssetDetail);
			}
			if (!AssetDetail.mPackagePath.IsEmpty())
			{
				OutAssetList.Add(AssetDetail);
			}
		}
		return true;
	}
	return false;
}


bool UFLibAssetManageHelperEx::GetRedirectorList(const TArray<FString>& InFilterPackagePaths, TArray<FAssetDetail>& OutRedirector)
{
	TArray<FAssetData> AllAssetData;
	if (UFLibAssetManageHelperEx::GetAssetsData(InFilterPackagePaths, AllAssetData,true))
	{
		for (const auto& AssetDataIndex : AllAssetData)
		{
			if (AssetDataIndex.IsValid() && AssetDataIndex.IsRedirector())
			{
				FAssetDetail AssetDetail;
				UFLibAssetManageHelperEx::ConvFAssetDataToFAssetDetail(AssetDataIndex, AssetDetail);
				OutRedirector.Add(AssetDetail);
			}
		}
		return true;
	}
	return false;
}

bool UFLibAssetManageHelperEx::GetSpecifyAssetData(const FString& InLongPackageName, TArray<FAssetData>& OutAssetData, bool InIncludeOnlyOnDiskAssets)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	return AssetRegistryModule.Get().GetAssetsByPackageName(*InLongPackageName, OutAssetData, InIncludeOnlyOnDiskAssets);
}

bool UFLibAssetManageHelperEx::GetAssetsData(const TArray<FString>& InFilterPackagePaths, TArray<FAssetData>& OutAssetData, bool bIncludeOnlyOnDiskAssets)
{
	OutAssetData.Reset();

	FARFilter Filter;
	Filter.bIncludeOnlyOnDiskAssets = bIncludeOnlyOnDiskAssets;
	Filter.bRecursivePaths = true;
	for(const auto& FilterPackageName: InFilterPackagePaths)
	{
		FString ValidFilterPackageName = FilterPackageName;
		while (ValidFilterPackageName.EndsWith("/"))
		{
			ValidFilterPackageName.RemoveAt(ValidFilterPackageName.Len() - 1);
		}
		Filter.PackagePaths.AddUnique(*ValidFilterPackageName);
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	AssetRegistryModule.Get().GetAssets(Filter, OutAssetData);
	
	return true;
}

bool UFLibAssetManageHelperEx::GetSingleAssetsData(const FString& InPackagePath, FAssetData& OutAssetData)
{
	UAssetManager& AssetManager = UAssetManager::Get();

	return AssetManager.GetAssetDataForPath(FSoftObjectPath{ InPackagePath }, OutAssetData);
}

bool UFLibAssetManageHelperEx::GetClassStringFromFAssetData(const FAssetData& InAssetData, FString& OutAssetType)
{
	bool bRunState = false;
	if (InAssetData.IsValid())
	{
		OutAssetType = InAssetData.AssetClass.ToString();
		bRunState = true;
	}
	return bRunState;
}


bool UFLibAssetManageHelperEx::ConvFAssetDataToFAssetDetail(const FAssetData& InAssetData, FAssetDetail& OutAssetDetail)
{
	if (!InAssetData.IsValid())
		return false;
	FAssetDetail AssetDetail;
	AssetDetail.mAssetType = InAssetData.AssetClass.ToString();
	UFLibAssetManageHelperEx::ConvLongPackageNameToPackagePath(InAssetData.PackageName.ToString(), AssetDetail.mPackagePath);

	UFLibAssetManageHelperEx::GetAssetPackageGUID(InAssetData.PackageName.ToString(), AssetDetail.mGuid);

	OutAssetDetail = AssetDetail;
	return !OutAssetDetail.mAssetType.IsEmpty() && !OutAssetDetail.mAssetType.IsEmpty() && !OutAssetDetail.mAssetType.IsEmpty();
}

bool UFLibAssetManageHelperEx::GetSpecifyAssetDetail(const FString& InLongPackageName, FAssetDetail& OutAssetDetail)
{

	bool bRunStatus = false;
	TArray<FAssetData> AssetData;
	if (UFLibAssetManageHelperEx::GetSpecifyAssetData(InLongPackageName, AssetData, true))
	{
		if (AssetData.Num() > 0)
		{
			UFLibAssetManageHelperEx::ConvFAssetDataToFAssetDetail(AssetData[0], OutAssetDetail);
			bRunStatus = true;
		}
	}
	return bRunStatus;
}

void UFLibAssetManageHelperEx::FilterNoRefAssets(const TArray<FAssetDetail>& InAssetsDetail, TArray<FAssetDetail>& OutHasRefAssetsDetail, TArray<FAssetDetail>& OutDontHasRefAssetsDetail)
{
	OutHasRefAssetsDetail.Reset();
	OutDontHasRefAssetsDetail.Reset();
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	for (const auto& AssetDetail : InAssetsDetail)
	{
		FSoftObjectPath CurrentObjectSoftRef{ AssetDetail.mPackagePath };
		FAssetIdentifier CurrentAssetId{ *CurrentObjectSoftRef.GetLongPackageName() };
		
		// ignore scan Map Asset reference
		{
			FAssetData CurrentAssetData;
			if (UFLibAssetManageHelperEx::GetSingleAssetsData(AssetDetail.mPackagePath, CurrentAssetData) && CurrentAssetData.IsValid())
			{
				if (CurrentAssetData.AssetClass == TEXT("World") || 
					CurrentAssetData.AssetClass == TEXT("MapBuildDataRegistry")
				)
				{
					OutHasRefAssetsDetail.Add(AssetDetail);
					continue;
				}
			}
		}

		
		TArray<FAssetIdentifier> CurrentAssetRefList;
		AssetRegistryModule.Get().GetReferencers(CurrentAssetId, CurrentAssetRefList);
		if (CurrentAssetRefList.Num() > 1 || (CurrentAssetRefList.Num() > 0 && !(CurrentAssetRefList[0] == CurrentAssetId)))
		{
			OutHasRefAssetsDetail.Add(AssetDetail);
		}
		else {
			OutDontHasRefAssetsDetail.Add(AssetDetail);
		}
	}
}

void UFLibAssetManageHelperEx::FilterNoRefAssetsWithIgnoreFilter(const TArray<FAssetDetail>& InAssetsDetail, const TArray<FString>& InIgnoreFilters, TArray<FAssetDetail>& OutHasRefAssetsDetail, TArray<FAssetDetail>& OutDontHasRefAssetsDetail)
{
	OutHasRefAssetsDetail.Reset();
	OutDontHasRefAssetsDetail.Reset();
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	for (const auto& AssetDetail : InAssetsDetail)
	{
		FSoftObjectPath CurrentObjectSoftRef{ AssetDetail.mPackagePath };
		FAssetIdentifier CurrentAssetId{ *CurrentObjectSoftRef.GetLongPackageName() };

		// ignore scan Map Asset reference
		{
			FAssetData CurrentAssetData;
			if (UFLibAssetManageHelperEx::GetSingleAssetsData(AssetDetail.mPackagePath, CurrentAssetData) && CurrentAssetData.IsValid())
			{
				if (CurrentAssetData.AssetClass == TEXT("World") ||
					CurrentAssetData.AssetClass == TEXT("MapBuildDataRegistry")
				)
				{
					bool bIsIgnoreAsset = false;
					for (const auto& IgnoreFilter : InIgnoreFilters)
					{
						if (CurrentAssetData.PackagePath.ToString().StartsWith(*IgnoreFilter))
						{
							bIsIgnoreAsset = true;
							break;
						}
					}

					if (!bIsIgnoreAsset)
					{
						OutHasRefAssetsDetail.Add(AssetDetail);
						continue;
					}
				}
			}
		}


		TArray<FAssetIdentifier> CurrentAssetRefList;
		AssetRegistryModule.Get().GetReferencers(CurrentAssetId, CurrentAssetRefList);
		if (CurrentAssetRefList.Num() > 1 || (CurrentAssetRefList.Num() > 0 && !(CurrentAssetRefList[0] == CurrentAssetId)))
		{
			OutHasRefAssetsDetail.Add(AssetDetail);
		}
		else {
			OutDontHasRefAssetsDetail.Add(AssetDetail);
		}
	}
}
bool UFLibAssetManageHelperEx::CombineAssetsDetailAsFAssetDepenInfo(const TArray<FAssetDetail>& InAssetsDetailList, FAssetDependenciesInfo& OutAssetInfo)
{
	FAssetDependenciesInfo result;

	for (const auto& AssetDetail : InAssetsDetailList)
	{
		FString CurrAssetModuleName = UFLibAssetManageHelperEx::GetAssetBelongModuleName(AssetDetail.mPackagePath);
		FSoftObjectPath CurrAssetObjectPath(AssetDetail.mPackagePath);
		FString CurrAssetLongPackageName = CurrAssetObjectPath.GetLongPackageName();
		if (!result.AssetsDependenciesMap.Contains(CurrAssetModuleName))
		{
			FAssetDependenciesDetail AssetDependenciesDetail{ CurrAssetModuleName,TMap<FString,FAssetDetail>{ {CurrAssetLongPackageName,AssetDetail} } };
			result.AssetsDependenciesMap.Add(CurrAssetModuleName, AssetDependenciesDetail);
		}
		else
		{
			FAssetDependenciesDetail& CurrentCategory = *result.AssetsDependenciesMap.Find(CurrAssetModuleName);
			
			if (!result.AssetsDependenciesMap.Contains(CurrAssetLongPackageName))
			{
				CurrentCategory.AssetDependencyDetails.Add(CurrAssetLongPackageName,AssetDetail);
			}
		}
	
	}
	OutAssetInfo = result;

	return true;
}

void UFLibAssetManageHelperEx::GetAllInValidAssetInProject(FAssetDependenciesInfo InAllDependencies, TArray<FString> &OutInValidAsset,TArray<FString> InIgnoreModules)
{
	TArray<FString> Keys;
	InAllDependencies.AssetsDependenciesMap.GetKeys(Keys);

	for (const auto& ModuleItem : Keys)
	{
		// ignore search /Script Asset
		if (InIgnoreModules.Contains(ModuleItem))
			continue;
		FAssetDependenciesDetail ModuleDependencies = InAllDependencies.AssetsDependenciesMap[ModuleItem];

		TArray<FString> ModuleAssetList;
		ModuleDependencies.AssetDependencyDetails.GetKeys(ModuleAssetList);
		for (const auto& AssetLongPackageName : ModuleAssetList)
		{
			FString AssetPackagePath;
			if(!UFLibAssetManageHelperEx::ConvLongPackageNameToPackagePath(AssetLongPackageName,AssetPackagePath))
				continue;
			// UE_LOG(LogAssetManagerEx, Log, TEXT("Asset %s"), *AssetPackagePath);
			FString AssetAbsPath = UFLibAssetManageHelperEx::ConvVirtualToAbsPath(AssetPackagePath);
			if (!FPaths::FileExists(AssetAbsPath))
			{
				OutInValidAsset.Add(AssetLongPackageName);
			}
		}
	}
}
const FAssetPackageData* UFLibAssetManageHelperEx::GetPackageDataByPackagePath(const FString& InPackagePath)
{

	if (InPackagePath.IsEmpty())
		return NULL;
	if (!InPackagePath.IsEmpty())
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		FString TargetLongPackageName = UFLibAssetManageHelperEx::GetLongPackageNameFromPackagePath(InPackagePath);

		if(FPackageName::DoesPackageExist(TargetLongPackageName))
		{
			FAssetPackageData* AssetPackageData = const_cast<FAssetPackageData*>(AssetRegistryModule.Get().GetAssetPackageData(*TargetLongPackageName));
			if (AssetPackageData != nullptr)
			{
				return AssetPackageData;
			}
		}
		else
		{
			UE_LOG(LogAssetManagerEx,Warning,TEXT("GetPackageDataByPackagePath %s Failed!"),*InPackagePath);
		}
	}

	return NULL;
}

bool UFLibAssetManageHelperEx::ConvLongPackageNameToCookedPath(const FString& InProjectAbsDir, const FString& InPlatformName, const FString& InLongPackageName, TArray<FString>& OutCookedAssetPath, TArray<FString>& OutCookedAssetRelativePath)
{
	if (!FPaths::DirectoryExists(InProjectAbsDir) || !IsValidPlatform(InPlatformName))
		return false;

	FString EngineAbsDir = FPaths::ConvertRelativePathToFull(FPaths::EngineDir());
	FString CookedRootDir = FPaths::Combine(InProjectAbsDir, TEXT("Saved/Cooked"), InPlatformName);
	FString ProjectName = FApp::GetProjectName();
	FString AssetPackagePath;
	if(!UFLibAssetManageHelperEx::ConvLongPackageNameToPackagePath(InLongPackageName,AssetPackagePath))
		return false;
	FString AssetAbsPath = UFLibAssetManageHelperEx::ConvVirtualToAbsPath(AssetPackagePath);

	FString AssetModuleName;
	GetModuleNameByRelativePath(InLongPackageName,AssetModuleName);

	bool bIsEngineModule = false;
	FString AssetBelongModuleBaseDir;
	FString AssetCookedNotPostfixPath;
	{

		if (UFLibAssetManageHelperEx::GetEnableModuleAbsDir(AssetModuleName, AssetBelongModuleBaseDir))
		{
			if (AssetBelongModuleBaseDir.Contains(EngineAbsDir))
				bIsEngineModule = true;
		}

		FString AssetCookedRelativePath;
		if (bIsEngineModule)
		{
			AssetCookedRelativePath = TEXT("Engine") / UKismetStringLibrary::GetSubstring(AssetAbsPath, EngineAbsDir.Len() - 1, AssetAbsPath.Len() - EngineAbsDir.Len());
		}
		else
		{
			AssetCookedRelativePath = ProjectName / UKismetStringLibrary::GetSubstring(AssetAbsPath, InProjectAbsDir.Len() - 1, AssetAbsPath.Len() - InProjectAbsDir.Len());
		}

		// remove .uasset / .umap postfix
		{
			int32 lastDotIndex = 0;
			AssetCookedRelativePath.FindLastChar('.', lastDotIndex);
			AssetCookedRelativePath.RemoveAt(lastDotIndex, AssetCookedRelativePath.Len() - lastDotIndex);
		}

		AssetCookedNotPostfixPath = FPaths::Combine(CookedRootDir, AssetCookedRelativePath);
	}

	FFileArrayDirectoryVisitor FileVisitor;
	FString SearchDir;
	{
		int32 lastSlashIndex;
		AssetCookedNotPostfixPath.FindLastChar('/', lastSlashIndex);
		SearchDir = UKismetStringLibrary::GetSubstring(AssetCookedNotPostfixPath, 0, lastSlashIndex);
	}
	IFileManager::Get().IterateDirectory(*SearchDir, FileVisitor);
	for (const auto& FileItem : FileVisitor.Files)
	{
		if (FileItem.Contains(AssetCookedNotPostfixPath) && FileItem[AssetCookedNotPostfixPath.Len()] == '.')
		{
			OutCookedAssetPath.Add(FileItem);
			{
				FString AssetCookedRelativePath = UKismetStringLibrary::GetSubstring(FileItem, CookedRootDir.Len() + 1, FileItem.Len() - CookedRootDir.Len());
				OutCookedAssetRelativePath.Add(FPaths::Combine(TEXT("../../../"), AssetCookedRelativePath));
			}
		}
	}
	return true;
}


bool UFLibAssetManageHelperEx::MakePakCommandFromAssetDependencies(
	const FString& InProjectDir, 
	const FString& InPlatformName, 
	const FAssetDependenciesInfo& InAssetDependencies, 
	//const TArray<FString> &InCookParams, 
	TArray<FString>& OutCookCommand, 
	TFunction<void(const TArray<FString>&,const TArray<FString>&, const FString&, const FString&)> InReceivePakCommand,
	TFunction<bool(const FString& CookedAssetsAbsPath)> IsIoStoreAsset
)
{
	if (!FPaths::DirectoryExists(InProjectDir) || !UFLibAssetManageHelperEx::IsValidPlatform(InPlatformName))
		return false;
	OutCookCommand.Empty();
	// TArray<FString> resault;
	TArray<FString> Keys;
	InAssetDependencies.AssetsDependenciesMap.GetKeys(Keys);

	for (const auto& Key : Keys)
	{
		if (Key.Equals(TEXT("Script")))
			continue;
		TArray<FString> AssetList;
		InAssetDependencies.AssetsDependenciesMap.Find(Key)->AssetDependencyDetails.GetKeys(AssetList);
		for (const auto& AssetLongPackageName : AssetList)
		{
			TArray<FString> FinalCookedCommand;
			if (UFLibAssetManageHelperEx::MakePakCommandFromLongPackageName(InProjectDir, InPlatformName, AssetLongPackageName, /*InCookParams, */FinalCookedCommand,InReceivePakCommand,IsIoStoreAsset))
			{
				OutCookCommand.Append(FinalCookedCommand);
			}
		}
	}
	return true;
}


bool UFLibAssetManageHelperEx::MakePakCommandFromLongPackageName(
	const FString& InProjectDir, 
	const FString& InPlatformName, 
	const FString& InAssetLongPackageName, 
	//const TArray<FString> &InCookParams, 
	TArray<FString>& OutCookCommand,
	TFunction<void(const TArray<FString>&,const TArray<FString>&, const FString&, const FString&)> InReceivePakCommand,
	TFunction<bool(const FString& CookedAssetsAbsPath)> IsIoStoreAsset
)
{
	OutCookCommand.Empty();
	bool bStatus = false;
	TArray<FString> CookedAssetAbsPath;
	TArray<FString> CookedAssetRelativePath;
	TArray<FString> FinalCookedPakCommand;
	TArray<FString> FinalCookedIoStoreCommand;
	if (UFLibAssetManageHelperEx::ConvLongPackageNameToCookedPath(InProjectDir, InPlatformName, InAssetLongPackageName, CookedAssetAbsPath, CookedAssetRelativePath))
	{
		if (UFLibAssetManageHelperEx::CombineCookedAssetCommand(CookedAssetAbsPath, CookedAssetRelativePath,/* InCookParams,*/ FinalCookedPakCommand,FinalCookedIoStoreCommand,IsIoStoreAsset))
		{
			if (!!CookedAssetRelativePath.Num() && !!FinalCookedPakCommand.Num())
			{
				InReceivePakCommand(FinalCookedPakCommand,FinalCookedIoStoreCommand, CookedAssetRelativePath[0],InAssetLongPackageName);
				OutCookCommand.Append(FinalCookedPakCommand);
				bStatus = true;
			}
		}
	}
	return bStatus;
}

bool UFLibAssetManageHelperEx::CombineCookedAssetCommand(
	const TArray<FString> &InAbsPath,
	const TArray<FString>& InRelativePath,
	//const TArray<FString>& InParams,
	TArray<FString>& OutPakCommand,
	TArray<FString>& OutIoStoreCommand,
	TFunction<bool(const FString& CookedAssetsAbsPath)> IsIoStoreAsset
)
{
	if (!(!!InAbsPath.Num() && !!InRelativePath.Num()))
	{
		return false;
	}

	OutPakCommand.Empty();
	if (InAbsPath.Num() != InRelativePath.Num())
		return false;
	int32 AssetNum = InAbsPath.Num();
	for (int32 index = 0; index < AssetNum; ++index)
	{
		if(InAbsPath[index].EndsWith(TEXT(".patch")))
			continue;;
		FString CurrentCommand = TEXT("\"") + InAbsPath[index] + TEXT("\" \"") + InRelativePath[index] + TEXT("\"");
		// for (const auto& Param : InParams)
		// {
		// 	CurrentCommand += TEXT(" ") + Param;
		// }
		if(!IsIoStoreAsset(InAbsPath[index]))
			OutPakCommand.Add(CurrentCommand);
		else
			OutIoStoreCommand.Add(CurrentCommand);
	}
	return true;
}

bool UFLibAssetManageHelperEx::ExportCookPakCommandToFile(const TArray<FString>& InCommand, const FString& InFile)
{
	return FFileHelper::SaveStringArrayToFile(InCommand, *InFile);
}

/*
	TOOLs Set Implementation
*/

bool UFLibAssetManageHelperEx::SerializeAssetDependenciesToJson(const FAssetDependenciesInfo& InAssetDependencies, FString& OutJsonStr)
{
	OutJsonStr.Empty();
	bool bRunStatus = false;
	TSharedPtr<FJsonObject> RootJsonObject;
	if (!UFLibAssetManageHelperEx::SerializeAssetDependenciesToJsonObject(InAssetDependencies, RootJsonObject))
		return bRunStatus;
	if (RootJsonObject.IsValid())
	{
		auto JsonWriter = TJsonWriterFactory<>::Create(&OutJsonStr);
		FJsonSerializer::Serialize(RootJsonObject.ToSharedRef(), JsonWriter);
		bRunStatus = true;
	}
	return bRunStatus;
}


bool UFLibAssetManageHelperEx::SerializeAssetDependenciesToJsonObject(const FAssetDependenciesInfo& InAssetDependencies, TSharedPtr<FJsonObject>& OutJsonObject, TArray<FString> InIgnoreModules)
{
	bool bRunStatus = false;
	if(!OutJsonObject.IsValid())
		OutJsonObject = MakeShareable(new FJsonObject);

	{
		// collect all module name
		TArray<FString> AssetCategoryList;
		InAssetDependencies.AssetsDependenciesMap.GetKeys(AssetCategoryList);		
		{
			TArray<TSharedPtr<FJsonValue>> JsonCategoryList;
			for (const auto& AssetCategoryItem : AssetCategoryList)
			{
				JsonCategoryList.Add(MakeShareable(new FJsonValueString(AssetCategoryItem)));
			}
			OutJsonObject->SetArrayField(JSON_MODULE_LIST_SECTION_NAME, JsonCategoryList);
		}


		// serialize asset list
		TSharedPtr<FJsonObject> AssetListJsonObject = MakeShareable(new FJsonObject);
		for (const auto& AssetCategoryItem : AssetCategoryList)
		{

			// TSharedPtr<FJsonObject> CategoryJsonObject = MakeShareable(new FJsonObject);

			TArray<TSharedPtr<FJsonValue>> CategoryAssetListJsonEntity;
			const FAssetDependenciesDetail& CategortItem = InAssetDependencies.AssetsDependenciesMap[AssetCategoryItem];
			TArray<TSharedPtr<FJsonValue>> AssetDetialsJsonObject;
			TArray<FString> CategoryAssetList;
			CategortItem.AssetDependencyDetails.GetKeys(CategoryAssetList);

			for (const auto& AssetItem : CategoryAssetList)
			{
				CategoryAssetListJsonEntity.Add(MakeShareable(new FJsonValueString(AssetItem)));

			}
			// CategoryJsonObject->SetArrayField(AssetCategoryItem, CategoryAssetListJsonEntity);
			AssetListJsonObject->SetArrayField(AssetCategoryItem, CategoryAssetListJsonEntity);

		}

		// collect all invalid asset
		{
			TArray<FString> AllInValidAssetList;
			UFLibAssetManageHelperEx::GetAllInValidAssetInProject(InAssetDependencies, AllInValidAssetList,InIgnoreModules);
			if (AllInValidAssetList.Num() > 0)
			{
				TArray<TSharedPtr<FJsonValue>> JsonInvalidAssetList;
				for (const auto& InValidAssetItem : AllInValidAssetList)
				{
					JsonInvalidAssetList.Add(MakeShareable(new FJsonValueString(InValidAssetItem)));
				}
				AssetListJsonObject->SetArrayField(JSON_ALL_INVALID_ASSET_SECTION_NAME, JsonInvalidAssetList);
			}
		}

		OutJsonObject->SetObjectField(JSON_ALL_ASSETS_LIST_SECTION_NAME, AssetListJsonObject);

		// serilize asset detail
		TSharedPtr<FJsonObject> AssetDetailsListJsonObject = MakeShareable(new FJsonObject);
		for (const auto& AssetCategoryItem : AssetCategoryList)
		{
			TSharedPtr<FJsonObject> CurrentCategoryJsonObject = MakeShareable(new FJsonObject);
			const FAssetDependenciesDetail& CategortItem = InAssetDependencies.AssetsDependenciesMap[AssetCategoryItem];
			TArray<FString> AssetList;
			CategortItem.AssetDependencyDetails.GetKeys(AssetList);
			for (const auto& AssetLongPackageName : AssetList)
			{
				const FAssetDetail& CurrentAssetDetail = *CategortItem.AssetDependencyDetails.Find(AssetLongPackageName);

				FString CurrentPackageName = FSoftObjectPath(CurrentAssetDetail.mPackagePath).GetLongPackageName();
				TSharedPtr<FJsonObject> CurrentJsonObject = UFLibAssetManageHelperEx::SerilizeAssetDetial(CurrentAssetDetail);
				CurrentCategoryJsonObject->SetObjectField(CurrentPackageName, CurrentJsonObject);
			}
			AssetDetailsListJsonObject->SetObjectField(AssetCategoryItem, CurrentCategoryJsonObject);
		}

		OutJsonObject->SetObjectField(JSON_ALL_ASSETS_Detail_SECTION_NAME, AssetDetailsListJsonObject);
	}
	return true;
}

bool UFLibAssetManageHelperEx::DeserializeAssetDependencies(const FString& InStream, FAssetDependenciesInfo& OutAssetDependencies)
{
	bool bRunStatus = false;
	if (InStream.IsEmpty()) return false;

	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(InStream);
	TSharedPtr<FJsonObject> RootJsonObject;;
	if (FJsonSerializer::Deserialize(JsonReader, RootJsonObject))
	{
		bRunStatus = UFLibAssetManageHelperEx::DeserializeAssetDependenciesForJsonObject(RootJsonObject, OutAssetDependencies);
	}
	return true;
}
  
bool UFLibAssetManageHelperEx::DeserializeAssetDependenciesForJsonObject(const TSharedPtr<FJsonObject>& InJsonObject, FAssetDependenciesInfo& OutAssetDependencies)
{
	bool bRunStatus = false;
	if (InJsonObject.IsValid())
	{
		TArray<TSharedPtr<FJsonValue>> JsonModuleList = InJsonObject->GetArrayField(JSON_MODULE_LIST_SECTION_NAME);
		for (const auto& JsonModuleItem : JsonModuleList)
		{

			FString ModuleName = JsonModuleItem->AsString();

			//deserialize assets list
			TArray<FString> AssetList;
			{
				TSharedPtr<FJsonObject> AssetsListJsonObject = InJsonObject->GetObjectField(JSON_ALL_ASSETS_LIST_SECTION_NAME);
				TArray<TSharedPtr<FJsonValue>> JsonAssetList = AssetsListJsonObject->GetArrayField(ModuleName);

				for (const auto& JsonAssetItem : JsonAssetList)
				{
					FString AssetInfo = JsonAssetItem->AsString();
					AssetList.Add(AssetInfo);
				}
			}
			// deserialize Assets detail
			TMap<FString,FAssetDetail> ModuleAssetDetail;
			{
				TSharedPtr<FJsonObject> AssetsDetailJsonObject = InJsonObject->GetObjectField(JSON_ALL_ASSETS_Detail_SECTION_NAME);
				TSharedPtr<FJsonObject> ModuleAssetDetailJsonObject = AssetsDetailJsonObject->GetObjectField(ModuleName);

				for (const auto& AssetLongPackageName : AssetList)
				{
					TSharedPtr<FJsonObject> CurrentAssetDetailJsonObject = ModuleAssetDetailJsonObject->GetObjectField(AssetLongPackageName);

					FAssetDetail CurrentAssetDetail;
					if (UFLibAssetManageHelperEx::DeserilizeAssetDetial(CurrentAssetDetailJsonObject, CurrentAssetDetail))
					{
						ModuleAssetDetail.Add(AssetLongPackageName,CurrentAssetDetail);
					}	
				}

			}
			OutAssetDependencies.AssetsDependenciesMap.Add(ModuleName, FAssetDependenciesDetail(ModuleName, ModuleAssetDetail));
		}

		bRunStatus = true;
	}

	return bRunStatus;
}

TSharedPtr<FJsonObject> UFLibAssetManageHelperEx::SerilizeAssetDetial(const FAssetDetail& InAssetDetail)
{
	TSharedPtr<FJsonObject> RootJsonObject = MakeShareable(new FJsonObject);
	{
		RootJsonObject->SetStringField("PackagePath", InAssetDetail.mPackagePath);
		RootJsonObject->SetStringField("AssetType", InAssetDetail.mAssetType);
		RootJsonObject->SetStringField("AssetGUID", InAssetDetail.mGuid);
	}

	return RootJsonObject;
}

bool UFLibAssetManageHelperEx::DeserilizeAssetDetial(TSharedPtr<FJsonObject>& InJsonObject, FAssetDetail& OutAssetDetail)
{
	bool bRunStatus = false;
	if (InJsonObject.IsValid())
	{
		OutAssetDetail.mPackagePath = InJsonObject->GetStringField(TEXT("PackagePath"));
		OutAssetDetail.mAssetType = InJsonObject->GetStringField(TEXT("AssetType"));
		OutAssetDetail.mGuid = InJsonObject->GetStringField(TEXT("AssetGUID"));
		bRunStatus = true;
	}
	return bRunStatus;
}

FString UFLibAssetManageHelperEx::SerializeAssetDetialArrayToString(const TArray<FAssetDetail>& InAssetDetialList)
{
	TSharedPtr<FJsonObject> RootJsonObject=MakeShareable(new FJsonObject);

	for (const auto& AssetDetial : InAssetDetialList)
	{
		RootJsonObject->SetObjectField(AssetDetial.mPackagePath, UFLibAssetManageHelperEx::SerilizeAssetDetial(AssetDetial));
	}

	FString OutJsonStr;
	auto JsonWriter = TJsonWriterFactory<>::Create(&OutJsonStr);
	FJsonSerializer::Serialize(RootJsonObject.ToSharedRef(), JsonWriter);
	return OutJsonStr;
}

bool UFLibAssetManageHelperEx::SaveStringToFile(const FString& InFile, const FString& InString)
{
	return FFileHelper::SaveStringToFile(InString, *InFile, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
}

bool UFLibAssetManageHelperEx::LoadFileToString(const FString& InFile, FString& OutString)
{
	return FFileHelper::LoadFileToString(OutString, *InFile);
}

bool UFLibAssetManageHelperEx::GetPluginModuleAbsDir(const FString& InPluginModuleName, FString& OutPath)
{
	bool bFindResault = false;
	TSharedPtr<IPlugin> FoundModule = IPluginManager::Get().FindPlugin(InPluginModuleName);

	if (FoundModule.IsValid())
	{
		bFindResault = true;
		OutPath = FPaths::ConvertRelativePathToFull(FoundModule->GetBaseDir());
	}
	return bFindResault;
}

void UFLibAssetManageHelperEx::GetAllEnabledModuleName(TMap<FString, FString>& OutModules)
{
	OutModules.Add(TEXT("Game"), FPaths::ProjectDir());
	OutModules.Add(TEXT("Engine"), FPaths::EngineDir());

	TArray<TSharedRef<IPlugin>> AllPlugin = IPluginManager::Get().GetEnabledPlugins();

	for (const auto& PluginItem : AllPlugin)
	{
		OutModules.Add(PluginItem.Get().GetName(), PluginItem.Get().GetBaseDir());
	}
}


bool UFLibAssetManageHelperEx::ModuleIsEnabled(const FString& InModuleName)
{
	TMap<FString,FString> AllEnabledModules;
	UFLibAssetManageHelperEx::GetAllEnabledModuleName(AllEnabledModules);
	TArray<FString> AllEnableModuleNames;
	AllEnabledModules.GetKeys(AllEnableModuleNames);
	return AllEnabledModules.Contains(InModuleName);
}

bool UFLibAssetManageHelperEx::GetModuleNameByRelativePath(const FString& InRelativePath, FString& OutModuleName)
{
	if (InRelativePath.IsEmpty()) return false;
	FString BelongModuleName = InRelativePath;
	{
		BelongModuleName.RemoveFromStart(TEXT("/"));
		int32 secondSlashIndex = -1;
		if(BelongModuleName.FindChar('/', secondSlashIndex))
			BelongModuleName = UKismetStringLibrary::GetSubstring(BelongModuleName, 0, secondSlashIndex);
	}
	OutModuleName = BelongModuleName;
	return true;
}

bool UFLibAssetManageHelperEx::GetEnableModuleAbsDir(const FString& InModuleName, FString& OutPath)
{
	if (InModuleName.Equals(TEXT("Engine")))
	{
		OutPath = FPaths::ConvertRelativePathToFull(FPaths::EngineDir());
		return true;
	}
	if (InModuleName.Equals(TEXT("Game")) || InModuleName.Equals(FApp::GetProjectName()))
	{
		OutPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
		return true;
	}
	return UFLibAssetManageHelperEx::GetPluginModuleAbsDir(InModuleName, OutPath);
}

FString UFLibAssetManageHelperEx::GetAssetBelongModuleName(const FString& InAssetRelativePath)
{
	FString LongDependentPackageName = InAssetRelativePath;

	int32 BelongModuleNameEndIndex = LongDependentPackageName.Find(TEXT("/"), ESearchCase::IgnoreCase, ESearchDir::FromStart, 1);
	FString BelongModuleName = UKismetStringLibrary::GetSubstring(LongDependentPackageName, 1, BelongModuleNameEndIndex - 1);// (LongDependentPackageName, BelongModuleNameEndIndex);

	return BelongModuleName;
}


bool UFLibAssetManageHelperEx::ConvRelativeDirToAbsDir(const FString& InRelativePath, FString& OutAbsPath)
{
	bool bRunStatus = false;
	FString BelongModuleName;
	if (UFLibAssetManageHelperEx::GetModuleNameByRelativePath(InRelativePath, BelongModuleName))
	{
		if (UFLibAssetManageHelperEx::ModuleIsEnabled(BelongModuleName))
		{
			FString ModuleAbsPath;
			if (!UFLibAssetManageHelperEx::GetEnableModuleAbsDir(BelongModuleName, ModuleAbsPath))
				return false;

			FString RelativeToModule = InRelativePath.Replace(*BelongModuleName, TEXT("Content"));
			RelativeToModule = RelativeToModule.StartsWith(TEXT("/")) ? RelativeToModule.Right(RelativeToModule.Len() - 1) : RelativeToModule;
			FString FinalFilterPath = FPaths::Combine(ModuleAbsPath, RelativeToModule);
			FPaths::MakeStandardFilename(FinalFilterPath);
			if (FPaths::DirectoryExists(FinalFilterPath))
			{
				OutAbsPath = FinalFilterPath;
				bRunStatus = true;
			}
		}
	}
	return bRunStatus;
}

bool UFLibAssetManageHelperEx::IsValidPlatform(const FString& PlatformName)
{
	for (const auto& PlatformItem : UFLibAssetManageHelperEx::GetAllTargetPlatform())
	{
		if (PlatformItem.Equals(PlatformName))
		{
			return true;
		}
	}
	return false;
}

TArray<FString> UFLibAssetManageHelperEx::GetAllTargetPlatform()
{
#ifdef __DEVELOPER_MODE__
	TArray<ITargetPlatform*> Platforms = GetTargetPlatformManager()->GetTargetPlatforms();
	TArray<FString> result;

	for (const auto& PlatformItem : Platforms)
	{
		result.Add(PlatformItem->PlatformName());
	}

#else
	TArray<FString> result = {
		"AllDesktop",
		"MacClient",
		"MacNoEditor",
		"MacServer",
		"Mac",
		"WindowsClient",
		"WindowsNoEditor",
		"WindowsServer",
		"Windows",
		"Android",
		"Android_ASTC",
		"Android_ATC",
		"Android_DXT",
		"Android_ETC1",
		"Android_ETC1a",
		"Android_ETC2",
		"Android_PVRTC",
		"AndroidClient",
		"Android_ASTCClient",
		"Android_ATCClient",
		"Android_DXTClient",
		"Android_ETC1Client",
		"Android_ETC1aClient",
		"Android_ETC2Client",
		"Android_PVRTCClient",
		"Android_Multi",
		"Android_MultiClient",
		"HTML5",
		"IOSClient",
		"IOS",
		"TVOSClient",
		"TVOS",
		"LinuxClient",
		"LinuxNoEditor",
		"LinuxServer",
		"Linux",
		"Lumin",
		"LuminClient"
	};

#endif
	return result;
}


bool UFLibAssetManageHelperEx::FindFilesRecursive(const FString& InStartDir, TArray<FString>& OutFileList, bool InRecursive)
{
	TArray<FString> CurrentFolderFileList;
	if (!FPaths::DirectoryExists(InStartDir))
		return false;

	FFileArrayDirectoryVisitor FileVisitor;
	IFileManager::Get().IterateDirectoryRecursively(*InStartDir, FileVisitor);

	OutFileList.Append(FileVisitor.Files);

	return true;
}

uint32 UFLibAssetManageHelperEx::ParserAssetDependenciesInfoNumber(const FAssetDependenciesInfo& AssetDependenciesInfo, TMap<FString, uint32> ModuleAssetsNumMaps)
{
	uint32 TotalAssetNum = 0;
	TArray<FString> ModuleCategorys;
	AssetDependenciesInfo.AssetsDependenciesMap.GetKeys(ModuleCategorys);
	for(const auto& ModuleKey:ModuleCategorys)
	{
		TArray<FString> AssetKeys;
		AssetDependenciesInfo.AssetsDependenciesMap.Find(ModuleKey)->AssetDependencyDetails.GetKeys(AssetKeys);
		ModuleAssetsNumMaps.Add(ModuleKey,AssetKeys.Num());
		TotalAssetNum+=AssetKeys.Num();
	}
	return TotalAssetNum;
}

FString UFLibAssetManageHelperEx::ParserModuleAssetsNumMap(const TMap<FString, uint32>& InMap)
{
	FString result;
	TArray<FString> Keys;
	InMap.GetKeys(Keys);

	for(const auto& Key:Keys)
	{
		result += FString::Printf(TEXT("%s\t%d\n"),*Key,*InMap.Find(Key));
	}
	return result;
}

FString UFLibAssetManageHelperEx::ConvPath_Slash2BackSlash(const FString& InPath)
{
	FString ResaultPath;
	TArray<FString> OutArray;

	InPath.ParseIntoArray(OutArray, TEXT("/"));
	int32 OutArrayNum = OutArray.Num();
	for (int32 Index = 0; Index < OutArrayNum; ++Index)
	{
		if (!OutArray[Index].IsEmpty() && Index < OutArrayNum - 1)/* && FPaths::DirectoryExists(ResaultPath + item)*/
		{
			ResaultPath.Append(OutArray[Index]);
			ResaultPath.Append(TEXT("\\"));
		}
		else {
			ResaultPath.Append(OutArray[Index]);
		}
	}
	return ResaultPath;
}

FString UFLibAssetManageHelperEx::ConvPath_BackSlash2Slash(const FString& InPath)
{
	FString ResaultPath;
	TArray<FString> OutArray;
	InPath.ParseIntoArray(OutArray, TEXT("\\"));
	if (OutArray.Num() == 1 && OutArray[0] == InPath)
	{
		InPath.ParseIntoArray(OutArray, TEXT("/"));
	}
	int32 OutArrayNum = OutArray.Num();
	for (int32 Index = 0; Index < OutArrayNum; ++Index)
	{
		if (!OutArray[Index].IsEmpty() && Index < OutArrayNum - 1)/* && FPaths::DirectoryExists(ResaultPath + item)*/
		{
			ResaultPath.Append(OutArray[Index]);
			ResaultPath.Append(TEXT("/"));
		}
		else {
			ResaultPath.Append(OutArray[Index]);
		}
	}
	return ResaultPath;
}


EAssetRegistryDependencyType::Type UFLibAssetManageHelperEx::ConvAssetRegistryDependencyToInternal(const EAssetRegistryDependencyTypeEx& InType)
{
	return static_cast<EAssetRegistryDependencyType::Type>((uint8)(InType));
}

void UFLibAssetManageHelperEx::GetAssetDataInPaths(const TArray<FString>& Paths, TArray<FAssetData>& OutAssetData)
{
	// Form a filter from the paths
	FARFilter Filter;
	Filter.bRecursivePaths = true;
	for (const FString& Path : Paths)
	{
		new (Filter.PackagePaths) FName(*Path);
	}
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	
	AssetRegistryModule.Get().GetAssets(Filter, OutAssetData);
}

PRAGMA_ENABLE_DEPRECATION_WARNINGS