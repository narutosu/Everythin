// Fill out your copyright notice in the Description page of Project Settings.


#include "FlibHotPatcherEditorHelper.h"
#include "CreatePatch/FExportPatchSettings.h"
#include "CreatePatch/FExportReleaseSettings.h"
#include "HotPatcherLog.h"

// engine header
#include <UObject/SavePackage.h>

#include "Interfaces/ITargetPlatform.h"
#include "HAL/PlatformFilemanager.h"
#include "Interfaces/ITargetPlatformManagerModule.h"
#include "Editor.h"
#include "GameDelegates.h"
#include "GameMapsSettings.h"
#include "HotPatcherEditor.h"
#include "IPlatformFileSandboxWrapper.h"
#include "PackageHelperFunctions.h"
#include "Async/Async.h"
#include "Engine/AssetManager.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/RedirectCollector.h"
#include "Misc/SecureHash.h"
#include "Serialization/ArrayWriter.h"
#include "Settings/ProjectPackagingSettings.h"

DEFINE_LOG_CATEGORY(LogHotPatcherEditorHelper);

TArray<FString> UFlibHotPatcherEditorHelper::GetAllCookOption()
{
	TArray<FString> result
	{
		"Iterate",
		"UnVersioned",
		"CookAll",
		"Compressed"
	};
	return result;
}

void UFlibHotPatcherEditorHelper::CreateSaveFileNotify(const FText& InMsg, const FString& InSavedFile,SNotificationItem::ECompletionState NotifyType)
{
	AsyncTask(ENamedThreads::GameThread,[InMsg,InSavedFile,NotifyType]()
	{
		auto Message = InMsg;
		FNotificationInfo Info(Message);
		Info.bFireAndForget = true;
		Info.ExpireDuration = 5.0f;
		Info.bUseSuccessFailIcons = false;
		Info.bUseLargeFont = false;

		const FString HyperLinkText = InSavedFile;
		Info.Hyperlink = FSimpleDelegate::CreateLambda(
			[](FString SourceFilePath)
			{
				FPlatformProcess::ExploreFolder(*SourceFilePath);
			},
			HyperLinkText
		);
		Info.HyperlinkText = FText::FromString(HyperLinkText);
		FSlateNotificationManager::Get().AddNotification(Info)->SetCompletionState(NotifyType);
	});
}

void UFlibHotPatcherEditorHelper::CheckInvalidCookFilesByAssetDependenciesInfo(
	const FString& InProjectAbsDir, 
	const FString& InPlatformName, 
	const FAssetDependenciesInfo& InAssetDependencies, 
	TArray<FAssetDetail>& OutValidAssets, 
	TArray<FAssetDetail>& OutInvalidAssets)
{
	OutValidAssets.Empty();
	OutInvalidAssets.Empty();
	TArray<FAssetDetail> AllAssetDetails;
	UFLibAssetManageHelperEx::GetAssetDetailsByAssetDependenciesInfo(InAssetDependencies,AllAssetDetails);

	for (const auto& AssetDetail : AllAssetDetails)
	{
		TArray<FString> CookedAssetPath;
		TArray<FString> CookedAssetRelativePath;
		FString AssetLongPackageName;
		if(!UFLibAssetManageHelperEx::ConvPackagePathToLongPackageName(AssetDetail.mPackagePath, AssetLongPackageName))
			continue;
		FAssetData CurrentAssetData;
		UFLibAssetManageHelperEx::GetSingleAssetsData(AssetDetail.mPackagePath,CurrentAssetData);
		if(!CurrentAssetData.GetAsset()->IsValidLowLevelFast())
		{
			UE_LOG(LogHotPatcherEditorHelper,Warning,TEXT("%s is invalid Asset Uobject"),*CurrentAssetData.PackageName.ToString());
			continue;
		}
		if (CurrentAssetData.GetAsset()->HasAnyMarks(OBJECTMARK_EditorOnly))
		{
			UE_LOG(LogHotPatcherEditorHelper,Warning,TEXT("Miss %s it's EditorOnly Assets!"),*CurrentAssetData.PackageName.ToString());
			continue;
		}
		
		if (UFLibAssetManageHelperEx::ConvLongPackageNameToCookedPath(
			InProjectAbsDir,
			InPlatformName,
			AssetLongPackageName,
			CookedAssetPath,
			CookedAssetRelativePath))
		{
			if (CookedAssetPath.Num() > 0)
			{
				OutValidAssets.Add(AssetDetail);
			}
			else
			{
				OutInvalidAssets.Add(AssetDetail);
			}
		}
	}
}


#include "Kismet/KismetSystemLibrary.h"

FChunkInfo UFlibHotPatcherEditorHelper::MakeChunkFromPatchSettings(const FExportPatchSettings* InPatchSetting)
{
	FChunkInfo Chunk;
	if (!(InPatchSetting && InPatchSetting))
	{
		return Chunk;
	}
	
	Chunk.ChunkName = InPatchSetting->VersionId;
	Chunk.bMonolithic = false;
	Chunk.MonolithicPathMode = EMonolithicPathMode::MountPath;
	Chunk.bStorageUnrealPakList = InPatchSetting->GetUnrealPakSettings().bStoragePakList;
	Chunk.bStorageIoStorePakList = InPatchSetting->GetIoStoreSettings().bStoragePakList;
	Chunk.AssetIncludeFilters = const_cast<FExportPatchSettings*>(InPatchSetting)->GetAssetIncludeFilters();
	Chunk.AssetIgnoreFilters = const_cast<FExportPatchSettings*>(InPatchSetting)->GetAssetIgnoreFilters();
	Chunk.bAnalysisFilterDependencies = InPatchSetting->IsAnalysisFilterDependencies();
	Chunk.IncludeSpecifyAssets = const_cast<FExportPatchSettings*>(InPatchSetting)->GetIncludeSpecifyAssets();
	Chunk.AddExternAssetsToPlatform = const_cast<FExportPatchSettings*>(InPatchSetting)->GetAddExternAssetsToPlatform();
	Chunk.AssetRegistryDependencyTypes = InPatchSetting->GetAssetRegistryDependencyTypes();
	Chunk.InternalFiles.bIncludeAssetRegistry = InPatchSetting->IsIncludeAssetRegistry();
	Chunk.InternalFiles.bIncludeGlobalShaderCache = InPatchSetting->IsIncludeGlobalShaderCache();
	Chunk.InternalFiles.bIncludeShaderBytecode = InPatchSetting->IsIncludeShaderBytecode();
	Chunk.InternalFiles.bIncludeEngineIni = InPatchSetting->IsIncludeEngineIni();
	Chunk.InternalFiles.bIncludePluginIni = InPatchSetting->IsIncludePluginIni();
	Chunk.InternalFiles.bIncludeProjectIni = InPatchSetting->IsIncludeProjectIni();

	return Chunk;
}

FChunkInfo UFlibHotPatcherEditorHelper::MakeChunkFromPatchVerison(const FHotPatcherVersion& InPatchVersion)
{
	FChunkInfo Chunk;
	Chunk.ChunkName = InPatchVersion.VersionId;
	Chunk.bMonolithic = false;
	Chunk.bStorageUnrealPakList = false;
	auto ConvPathStrToDirPaths = [](const TArray<FString>& InPathsStr)->TArray<FDirectoryPath>
	{
		TArray<FDirectoryPath> result;
		for (const auto& Dir : InPathsStr)
		{
			FDirectoryPath Path;
			Path.Path = Dir;
			result.Add(Path);
		}
		return result;
	};

	//Chunk.AssetIncludeFilters = ConvPathStrToDirPaths(InPatchVersion.IgnoreFilter);
	// Chunk.AssetIgnoreFilters = ConvPathStrToDirPaths(InPatchVersion.IgnoreFilter);
	Chunk.bAnalysisFilterDependencies = false;
	TArray<FAssetDetail> AllVersionAssets;
	UFLibAssetManageHelperEx::GetAssetDetailsByAssetDependenciesInfo(InPatchVersion.AssetInfo, AllVersionAssets);

	for (const auto& Asset : AllVersionAssets)
	{
		FPatcherSpecifyAsset CurrentAsset;
		CurrentAsset.Asset = FSoftObjectPath(Asset.mPackagePath);
		CurrentAsset.bAnalysisAssetDependencies = false;
		Chunk.IncludeSpecifyAssets.AddUnique(CurrentAsset);
	}
	// Chunk.AddExternDirectoryToPak = InPatchSetting->GetAddExternDirectory();
	// for (const auto& File : InPatchVersion.ExternalFiles)
	// {
	// 	Chunk.AddExternFileToPak.AddUnique(File.Value);
	// }

	TArray<ETargetPlatform> VersionPlatforms;

	InPatchVersion.PlatformAssets.GetKeys(VersionPlatforms);

	for(auto Platform:VersionPlatforms)
	{
		Chunk.AddExternAssetsToPlatform.Add(InPatchVersion.PlatformAssets[Platform]);
	}
	
	Chunk.InternalFiles.bIncludeAssetRegistry = false;
	Chunk.InternalFiles.bIncludeGlobalShaderCache = false;
	Chunk.InternalFiles.bIncludeShaderBytecode = false;
	Chunk.InternalFiles.bIncludeEngineIni = false;
	Chunk.InternalFiles.bIncludePluginIni = false;
	Chunk.InternalFiles.bIncludeProjectIni = false;

	return Chunk;
}

#define REMAPPED_PLUGGINS TEXT("RemappedPlugins")


FString ConvertToFullSandboxPath( const FString &FileName, bool bForWrite )
{
	FString Result;
	FString ProjectContentAbsir = FPaths::ConvertRelativePathToFull(FPaths::ProjectContentDir());
	if(FileName.StartsWith(ProjectContentAbsir))
	{
		FString GameFileName = FileName;
		GameFileName.RemoveFromStart(ProjectContentAbsir);
		Result = FPaths::Combine(FApp::GetProjectName(),TEXT("Content"),GameFileName);
		return Result;
	}
	if(FileName.StartsWith(FPaths::EngineContentDir()))
	{
		FString EngineFileName = FileName;
		EngineFileName.RemoveFromStart(FPaths::EngineContentDir());
		Result = FPaths::Combine(TEXT("Engine/Content"),EngineFileName);
		return Result;
	}
	TArray<TSharedRef<IPlugin> > PluginsToRemap = IPluginManager::Get().GetEnabledPlugins();
	// Ideally this would be in the Sandbox File but it can't access the project or plugin
	if (PluginsToRemap.Num() > 0)
	{
		// Handle remapping of plugins
		for (TSharedRef<IPlugin> Plugin : PluginsToRemap)
		{
			FString PluginContentDir;
			if (FPaths::IsRelative(FileName))
				PluginContentDir = Plugin->GetContentDir();
			else
				PluginContentDir = FPaths::ConvertRelativePathToFull(Plugin->GetContentDir());
			// UE_LOG(LogHotPatcherEditorHelper,Log,TEXT("Plugin Content:%s"),*PluginContentDir);
			if (FileName.StartsWith(PluginContentDir))
			{
				FString LoadingFrom;
				FString BasePath;
				switch(Plugin->GetLoadedFrom())
				{
				case EPluginLoadedFrom::Engine:
					{
						BasePath = FPaths::ConvertRelativePathToFull(FPaths::EngineDir());//TEXT("Engine/Plugins");
						LoadingFrom = TEXT("Engine");
						break;
					}
				case EPluginLoadedFrom::Project:
					{
						BasePath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());//FPaths::Combine(FApp::GetProjectName(),TEXT("Plugins"));
						LoadingFrom = FApp::GetProjectName();
						break;
					}
				}
				FString AssetAbsPath = FPaths::ConvertRelativePathToFull(FileName);
				if(AssetAbsPath.StartsWith(BasePath))
				{
					Result = LoadingFrom / AssetAbsPath.RightChop(BasePath.Len());
				}
			}
		}
	}

	return Result;
}

FString UFlibHotPatcherEditorHelper::GetCookAssetsSaveDir(const FString& BaseDir, const FString PacakgeName, const FString& Platform)
{
	FString Filename;
	FString PackageFilename;
	FString StandardFilename;
	FName StandardFileFName = NAME_None;

	if (FPackageName::DoesPackageExist(PacakgeName, NULL, &Filename, false))
	{
		StandardFilename = PackageFilename = FPaths::ConvertRelativePathToFull(Filename);

		FPaths::MakeStandardFilename(StandardFilename);
		StandardFileFName = FName(*StandardFilename);
	}

	FString SandboxFilename = ConvertToFullSandboxPath(*StandardFilename, true);
	// UE_LOG(LogHotPatcherEditorHelper,Log,TEXT("Filename:%s,PackageFileName:%s,StandardFileName:%s"),*Filename,*PackageFilename,*StandardFilename);
	
	FString CookDir =FPaths::Combine(BaseDir,Platform,SandboxFilename);
	
	return 	CookDir;
}

FString UFlibHotPatcherEditorHelper::GetProjectCookedDir()
{
	return FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("Cooked")));;
}

#if WITH_PACKAGE_CONTEXT
// engine header
#include "UObject/SavePackage.h"

#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION > 25
#include "Serialization/BulkDataManifest.h"
#endif

#if ENGINE_MAJOR_VERSION > 4 && ENGINE_MINOR_VERSION > 0 
#include "CookerWriterForUE5/CookerWriterForUE5.h"
#endif

#endif

FSavePackageContext* UFlibHotPatcherEditorHelper::CreateSaveContext(const ITargetPlatform* TargetPlatform,
	bool bUseZenLoader)
{
	FSavePackageContext* SavePackageContext = NULL;
#if WITH_PACKAGE_CONTEXT
	const FString PlatformString = TargetPlatform->PlatformName();

	// const FString ResolvedRootPath = RootPathSandbox.Replace(TEXT("[Platform]"), *PlatformString);
	const FString ResolvedProjectPath = FPaths::Combine(FPaths::ProjectDir(),FString::Printf(TEXT("Saved/Cooked/%s/%s"),*TargetPlatform->PlatformName(),FApp::GetProjectName()));
	const FString ResolvedMetadataPath = FPaths::Combine(ResolvedProjectPath,TEXT("Mededata"));
	
	FConfigFile PlatformEngineIni;
	FConfigCacheIni::LoadLocalIniFile(PlatformEngineIni, TEXT("Engine"), true, *TargetPlatform->IniPlatformName());
	
	
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION > 25
	FPackageStoreBulkDataManifest* BulkDataManifest	= new FPackageStoreBulkDataManifest(ResolvedProjectPath);
	FLooseFileWriter* LooseFileWriter				= bUseZenLoader ? new FLooseFileWriter() : nullptr;
	bool bLegacyBulkDataOffsets = false;
	PlatformEngineIni.GetBool(TEXT("Core.System"), TEXT("LegacyBulkDataOffsets"), bLegacyBulkDataOffsets);
	SavePackageContext	= new FSavePackageContext(LooseFileWriter, BulkDataManifest, bLegacyBulkDataOffsets);
#endif
	
#if ENGINE_MAJOR_VERSION > 4 && ENGINE_MINOR_VERSION > 0 
	ICookedPackageWriter* PackageWriter = nullptr;
	FString WriterDebugName;
	if (bUseZenLoader)
	{
		PackageWriter = new FZenStoreWriter(ResolvedProjectPath, ResolvedMetadataPath, TargetPlatform);
		WriterDebugName = TEXT("ZenStore");
	}
	else
	{
		FAsyncIODelete AsyncIODelete{ResolvedProjectPath};
		PackageWriter = new FLooseCookedPackageWriter(ResolvedProjectPath, ResolvedMetadataPath, TargetPlatform,AsyncIODelete,FPackageNameCache{},IPluginManager::Get().GetEnabledPlugins());
		WriterDebugName = TEXT("DirectoryWriter");
	}
	
	SavePackageContext	= new FSavePackageContext(TargetPlatform, PackageWriter);
#endif
#endif
	return SavePackageContext;
}


bool UFlibHotPatcherEditorHelper::CookAssets(
	const TArray<FSoftObjectPath>& Assets,
	const TArray<ETargetPlatform>&Platforms,
	TFunction<void(const FString&)> PackageSavedCallback,
	TFunction<void(const FString&,ETargetPlatform)> CookFailedCallback,
    class TMap<ETargetPlatform,FSavePackageContext*> PlatformSavePackageContext
)
{
	TArray<FAssetData> AssetsData;
	TArray<UPackage*> Packages;
	for(const auto& Asset:Assets)
	{
		FAssetData AssetData;
		if(UFLibAssetManageHelperEx::GetSingleAssetsData(Asset.GetAssetPathString(),AssetData))
		{
			AssetsData.AddUnique(AssetData);
			Packages.AddUnique(AssetData.GetPackage());
		}
	}
	TArray<FString> StringPlatforms;
	TMap<FString,FSavePackageContext*> FinalPlatformSavePackageContext;
	for(const auto& Platform:Platforms)
	{
		FString PlatformName = UFlibPatchParserHelper::GetEnumNameByValue(Platform);
		StringPlatforms.AddUnique(PlatformName);
		FSavePackageContext* CurrentPackageContext = NULL;
		if(PlatformSavePackageContext.Contains(Platform))
		{
			CurrentPackageContext = *PlatformSavePackageContext.Find(Platform);
		}
		 
		FinalPlatformSavePackageContext.Add(PlatformName,CurrentPackageContext);
	}
	
	return CookPackages(AssetsData,Packages,StringPlatforms,PackageSavedCallback,CookFailedCallback,FinalPlatformSavePackageContext);
}

bool UFlibHotPatcherEditorHelper::CookPackages(const TArray<FAssetData>& AssetDatas, const TArray<UPackage*>& InPackage,
	const TArray<FString>& Platforms, TFunction<void(const FString&)> PackageSavedCallback,
	TFunction<void(const FString&, ETargetPlatform)> CookFailedCallback,
	TMap<ETargetPlatform, FSavePackageContext*> PlatformSavePackageContext)
{
	TMap<FString,FSavePackageContext*> PlatformStrSavePackageContext;
	for(auto& PlatformContext:PlatformSavePackageContext)
	{
		FString PlatformName = UFlibPatchParserHelper::GetEnumNameByValue(PlatformContext.Key);
		PlatformStrSavePackageContext.Add(PlatformName,PlatformContext.Value);
	}
	return UFlibHotPatcherEditorHelper::CookPackages(AssetDatas,InPackage,Platforms,PackageSavedCallback,CookFailedCallback,PlatformStrSavePackageContext);
}

bool UFlibHotPatcherEditorHelper::CookPackages(
	const TArray<FAssetData>& AssetDatas,
	const TArray<UPackage*>& InPackage,
	const TArray<FString>& Platforms,
	TFunction<void(const FString&)> PackageSavedCallback,
	TFunction<void(const FString&,ETargetPlatform)> CookFailedCallback,
	class TMap<FString,FSavePackageContext*> PlatformSavePackageContext
)
{
	if(AssetDatas.Num()!=InPackage.Num())
		return false;
	for(int32 index=0;index<InPackage.Num();++index)
	{
		CookPackage(AssetDatas[index],InPackage[index],Platforms,PackageSavedCallback,CookFailedCallback,PlatformSavePackageContext);
	}
	return true;
}

bool UFlibHotPatcherEditorHelper::CookPackage(
	const FAssetData& AssetData,
	UPackage* Package,
	const TArray<FString>& Platforms,
	//const FString& SavePath,
	TFunction<void(const FString&)> PackageSavedCallback,
	TFunction<void(const FString&,ETargetPlatform)> CookFailedCallback,
	class TMap<FString,FSavePackageContext*> PlatformSavePackageContext
)
{
	bool bSuccessed = false;
	const bool bStorageConcurrent = FParse::Param(FCommandLine::Get(), TEXT("ConcurrentSave"));
	bool bUnversioned = true;
	uint32 SaveFlags = SAVE_KeepGUID | SAVE_Async| SAVE_ComputeHash | (bUnversioned ? SAVE_Unversioned : 0);

#if ENGINE_MAJOR_VERSION >4 || ENGINE_MINOR_VERSION >25
	bool CookLinkerDiff = false;
	if(CookLinkerDiff)
	{
		SaveFlags |= SAVE_CompareLinker;
	}
#endif
	
	EObjectFlags CookedFlags = RF_Public;
	if(UWorld::FindWorldInPackage(Package))
	{
		CookedFlags = RF_NoFlags;
	}
	if (bStorageConcurrent)
	{
		SaveFlags |= SAVE_Concurrent;
	}
	TArray<ITargetPlatform*> CookPlatforms; 
	for (auto const& PlatformName : Platforms)
	{
		ITargetPlatform* Platform = GetPlatformByName(PlatformName);
		if(Platform)
		{
			CookPlatforms.Add(Platform);
		}
	}
	FString SavePath = FPaths::Combine(FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir()),TEXT("Cooked"));
#if ENGINE_MAJOR_VERSION > 4
	FName PackageFileName = Package->GetLoadedPath().GetPackageFName();
#else
	FName PackageFileName = Package->FileName;
#endif
	if(PackageFileName.IsNone() && AssetData.PackageName.IsNone())
		return bSuccessed;
	for(auto& Platform:CookPlatforms)
	{
		struct FFilterEditorOnlyFlag
		{
			FFilterEditorOnlyFlag(UPackage* InPackage,ITargetPlatform* InPlatform)
			{
				Package = InPackage;
				Platform = InPlatform;
				if(!Platform->HasEditorOnlyData())
				{
					Package->SetPackageFlags(PKG_FilterEditorOnly);
				}
				else
				{
					Package->ClearPackageFlags(PKG_FilterEditorOnly);
				}
			}
			~FFilterEditorOnlyFlag()
			{
				if(!Platform->HasEditorOnlyData())
				{
					Package->ClearPackageFlags(PKG_FilterEditorOnly);
				}
			}
			UPackage* Package;
			ITargetPlatform* Platform;
		};

		FFilterEditorOnlyFlag SetPackageEditorOnlyFlag(Package,Platform);

		FString PackageName = PackageFileName.IsNone()?AssetData.PackageName.ToString():PackageFileName.ToString();
		FString CookedSavePath = UFlibHotPatcherEditorHelper::GetCookAssetsSaveDir(SavePath,PackageName, Platform->PlatformName());
		ETargetPlatform TargetPlatform;
		UFlibPatchParserHelper::GetEnumValueByName(Platform->PlatformName(),TargetPlatform);
		
		// delete old cooked assets
		if(FPaths::FileExists(CookedSavePath))
		{
			IFileManager::Get().Delete(*CookedSavePath);
		}
		// UE_LOG(LogHotPatcherEditorHelper,Log,TEXT("Cook Assets:%s"),*Package->GetName());
		Package->FullyLoad();
		TArray<UObject*> ExportMap;
		GetObjectsWithOuter(Package,ExportMap);
		for(const auto& ExportObj:ExportMap)
		{
			ExportObj->BeginCacheForCookedPlatformData(Platform);
		}

		// if(!bStorageConcurrent)
		// {
		// 	TArray<UObject*> TagExpObjects;
		// 	GetObjectsWithAnyMarks(TagExpObjects,OBJECTMARK_TagExp);
		// 	for(const auto& TagExportObj:TagExpObjects)
		// 	{
		// 		if(TagExportObj->HasAnyMarks(OBJECTMARK_TagExp))
		// 		{
		// 			TagExportObj->BeginCacheForCookedPlatformData(Platform);
		// 		}
		// 	}
		// }
		if(GCookLog)
		{
			UE_LOG(LogHotPatcher,Log,TEXT("Cook %s for %s"),*Package->GetName(),*Platform->PlatformName());
		}
#if WITH_PACKAGE_CONTEXT
		FSavePackageContext* CurrentPlatformPackageContext = nullptr;
		if(PlatformSavePackageContext.Contains(Platform->PlatformName()))
		{
			CurrentPlatformPackageContext = *PlatformSavePackageContext.Find(Platform->PlatformName());
		}
	#if ENGINE_MAJOR_VERSION > 4 && ENGINE_MINOR_VERSION > 0
			IPackageWriter::FBeginPackageInfo BeginInfo;
			BeginInfo.PackageName = Package->GetFName();
			BeginInfo.LooseFilePath = CookedSavePath;
			CurrentPlatformPackageContext->PackageWriter->BeginPackage(BeginInfo);
	#endif
#endif
		GIsCookerLoadingPackage = true;
		UPackage::PackageSavedEvent.AddLambda([PackageSavedCallback](const FString& InFilePath,UObject* Object){PackageSavedCallback(InFilePath);});
		
		// UE_LOG(LogHotPatcherEditorHelper,Display,TEXT("Cook Assets:%s save to %s"),*Package->GetName(),*CookedSavePath);
		FSavePackageResultStruct Result = GEditor->Save(	Package, nullptr, CookedFlags, *CookedSavePath, 
                                                GError, nullptr, false, false, SaveFlags, Platform, 
                                                FDateTime::MinValue(), false, /*DiffMap*/ nullptr
#if WITH_PACKAGE_CONTEXT
                                                ,CurrentPlatformPackageContext
#endif
                                                );
		GIsCookerLoadingPackage = false;
		bSuccessed = Result == ESavePackageResult::Success;
#if WITH_PACKAGE_CONTEXT
		// in UE5.1
	#if ENGINE_MAJOR_VERSION > 4 && ENGINE_MINOR_VERSION > 0
			// save cooked file to desk in UE5-main
			if(bSuccessed)
			{
				const FAssetPackageData* AssetPackageData = UFLibAssetManageHelperEx::GetPackageDataByPackagePath(Package->GetFName().ToString());
				ICookedPackageWriter::FCommitPackageInfo Info;
				Info.bSucceeded = bSuccessed;
				Info.PackageName = Package->GetFName();
				PRAGMA_DISABLE_DEPRECATION_WARNINGS
				Info.PackageGuid = AssetPackageData->PackageGuid;
				PRAGMA_ENABLE_DEPRECATION_WARNINGS
				// Info.Attachments.Add({ "Dependencies", TargetDomainDependencies });
				// TODO: Reenable BuildDefinitionList once FCbPackage support for empty FCbObjects is in
				//Info.Attachments.Add({ "BuildDefinitionList", BuildDefinitionList });
				Info.WriteOptions = IPackageWriter::EWriteOptions::Write;
				if (!!(SaveFlags & SAVE_ComputeHash))
				{
					Info.WriteOptions |= IPackageWriter::EWriteOptions::ComputeHash;
				}
				CurrentPlatformPackageContext->PackageWriter->CommitPackage(MoveTemp(Info));
			}
	#endif
#endif

		if(!bSuccessed && !Package->HasAnyPackageFlags(PKG_FilterEditorOnly))
		{
			CookFailedCallback(PackageName,TargetPlatform);
		}
	}
	return bSuccessed;
}

void UFlibHotPatcherEditorHelper::CookChunkAssets(
	TArray<FAssetDetail> Assets,
	const TArray<ETargetPlatform>& Platforms,
	TFunction<void(const FString&,ETargetPlatform)> CookFailedCallback,
	class TMap<ETargetPlatform,FSavePackageContext*> PlatformSavePackageContext
)
{
	TArray<FSoftObjectPath> AssetsSoftPath;

	for(const auto& Asset:Assets)
	{
		FSoftObjectPath AssetSoftPath;
		AssetSoftPath.SetPath(Asset.mPackagePath);
		AssetsSoftPath.AddUnique(AssetSoftPath);
	}
	if(!!AssetsSoftPath.Num())
	{
		UFlibHotPatcherEditorHelper::CookAssets(AssetsSoftPath,Platforms,[](const FString&){},CookFailedCallback,PlatformSavePackageContext);
	}
}

ITargetPlatform* UFlibHotPatcherEditorHelper::GetTargetPlatformByName(const FString& PlatformName)
{
	ITargetPlatformManagerModule& TPM = GetTargetPlatformManagerRef();
	const TArray<ITargetPlatform*>& TargetPlatforms = TPM.GetTargetPlatforms();
	ITargetPlatform* PlatformIns = NULL; 
	for (ITargetPlatform *TargetPlatform : TargetPlatforms)
	{
		if (PlatformName.Equals(TargetPlatform->PlatformName()))
		{
			PlatformIns = TargetPlatform;
		}
	}
	return PlatformIns;
}

TArray<ITargetPlatform*> UFlibHotPatcherEditorHelper::GetTargetPlatformsByNames(const TArray<ETargetPlatform>& Platforms)
{

	TArray<ITargetPlatform*> result;
	for(const auto& Platform:Platforms)
	{
    			
		ITargetPlatform* Found = UFlibHotPatcherEditorHelper::GetTargetPlatformByName(UFlibPatchParserHelper::GetEnumNameByValue(Platform,false));
		if(Found)
		{
			result.Add(Found);
		}
	}
	return result;
}



FString UFlibHotPatcherEditorHelper::GetUnrealPakBinary()
{
#if PLATFORM_WINDOWS
	return FPaths::Combine(
        FPaths::ConvertRelativePathToFull(FPaths::EngineDir()),
        TEXT("Binaries"),
#if PLATFORM_64BITS	
        TEXT("Win64"),
#else
        TEXT("Win32"),
#endif
        TEXT("UnrealPak.exe")
    );
#endif

#if PLATFORM_MAC
	return FPaths::Combine(
            FPaths::ConvertRelativePathToFull(FPaths::EngineDir()),
            TEXT("Binaries"),
            TEXT("Mac"),
            TEXT("UnrealPak")
    );
#endif

	return TEXT("");
}

FString UFlibHotPatcherEditorHelper::GetUECmdBinary()
{
	FString Binary;
#if ENGINE_MAJOR_VERSION > 4
	Binary = TEXT("UnrealEditor");
#else
	Binary = TEXT("UE4Editor");
#endif

	
#if PLATFORM_WINDOWS
	return FPaths::Combine(
        FPaths::ConvertRelativePathToFull(FPaths::EngineDir()),
        TEXT("Binaries"),
#if PLATFORM_64BITS	
        TEXT("Win64"),
#else
        TEXT("Win32"),
#endif
#ifdef WITH_HOTPATCHER_DEBUGGAME
	#if PLATFORM_64BITS
			FString::Printf(TEXT("%s-Win64-DebugGame-Cmd.exe"),*Binary)
	        // TEXT("UE4Editor-Win64-DebugGame-Cmd.exe")
	#else
			FString::Printf(TEXT("%s-Win32-DebugGame-Cmd.exe"),*Binary)
	        // TEXT("UE4Editor-Win32-DebugGame-Cmd.exe")
	#endif
#else
		FString::Printf(TEXT("%s-Cmd.exe"),*Binary)
        // TEXT("UE4Editor-Cmd.exe")
#endif
    );
#endif
#if PLATFORM_MAC
	return FPaths::Combine(
        FPaths::ConvertRelativePathToFull(FPaths::EngineDir()),
        TEXT("Binaries"),
        TEXT("Mac"),
        FString::Printf(TEXT("%s-Cmd"),*Binary)
        //TEXT("UE4Editor-Cmd")
    );
#endif
	return TEXT("");
}


FProcHandle UFlibHotPatcherEditorHelper::DoUnrealPak(TArray<FString> UnrealPakCommandletOptions, bool block)
{
	FString UnrealPakBinary = UFlibHotPatcherEditorHelper::GetUnrealPakBinary();

	FString CommandLine;
	for (const auto& Option : UnrealPakCommandletOptions)
	{
		CommandLine.Append(FString::Printf(TEXT(" %s"), *Option));
	}

	// create UnrealPak process

	uint32 *ProcessID = NULL;
	FProcHandle ProcHandle = FPlatformProcess::CreateProc(*UnrealPakBinary, *CommandLine, true, false, false, ProcessID, 0, NULL, NULL, NULL);

	if (ProcHandle.IsValid())
	{
		if (block)
		{
			FPlatformProcess::WaitForProc(ProcHandle);
		}
	}
	return ProcHandle;
}

FString UFlibHotPatcherEditorHelper::GetMetadataDir(const FString& ProjectDir, const FString& ProjectName,ETargetPlatform Platform)
{
	FString result;
	FString PlatformName = UFlibPatchParserHelper::GetEnumNameByValue(Platform,false);
	return FPaths::Combine(ProjectDir,TEXT("Saved/Cooked"),PlatformName,ProjectName,TEXT("Metadata"));
}

void UFlibHotPatcherEditorHelper::BackupMetadataDir(const FString& ProjectDir, const FString& ProjectName,
	const TArray<ETargetPlatform>& Platforms, const FString& OutDir)
{
	IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
	for(const auto& Platform:Platforms)
	{
		FString MetadataDir = FPaths::ConvertRelativePathToFull(UFlibHotPatcherEditorHelper::GetMetadataDir(ProjectDir,ProjectName,Platform));
		FString OutMetadir = FPaths::Combine(OutDir,TEXT("BackupMatedatas"),UFlibPatchParserHelper::GetEnumNameByValue(Platform,false));
		if(FPaths::DirectoryExists(MetadataDir))
		{
			PlatformFile.CreateDirectoryTree(*OutMetadir);
			PlatformFile.CopyDirectoryTree(*OutMetadir,*MetadataDir,true);
		}
	}
}

FString UFlibHotPatcherEditorHelper::ReleaseSummary(const FHotPatcherVersion& NewVersion)
{
	FString result = TEXT("\n---------------------HotPatcher Release Summary---------------------\n");

	auto ParserPlatformExternAssets = [](ETargetPlatform Platform,const FPlatformExternAssets& PlatformExternAssets)->FString
	{
		FString PlatformName = UFlibPatchParserHelper::GetEnumNameByValue(Platform);;
		FString result = FString::Printf(TEXT("======== %s Non-Assets =========\n"),*PlatformName);
		uint32 BeginLength = result.Len();
		result += FString::Printf(TEXT("External Files: %d\n"),PlatformExternAssets.AddExternFileToPak.Num());
		// result += FString::Printf(TEXT("External Directorys: %d\n"),PlatformExternAssets.AddExternDirectoryToPak.Num());

		for(uint32 index=0;index<BeginLength;++index)
			result += TEXT("=");
		result+=TEXT("\n");
		return result;
	};
	TMap<FString,uint32> AddModuleAssetNumMap;
	result += FString::Printf(TEXT("New Release Asset Number: %d\n"),UFLibAssetManageHelperEx::ParserAssetDependenciesInfoNumber(NewVersion.AssetInfo,AddModuleAssetNumMap));
	result += UFLibAssetManageHelperEx::ParserModuleAssetsNumMap(AddModuleAssetNumMap);
	TArray<ETargetPlatform> Keys;
	NewVersion.PlatformAssets.GetKeys(Keys);
	for(const auto& Key:Keys)
	{
		result += ParserPlatformExternAssets(Key,*NewVersion.PlatformAssets.Find(Key));
	}
	return result;
}

FString UFlibHotPatcherEditorHelper::PatchSummary(const FPatchVersionDiff& DiffInfo)
{
	auto ExternFileSummary = [](ETargetPlatform Platform,const FPatchVersionExternDiff& ExternDiff)->FString
	{
		FString PlatformName = UFlibPatchParserHelper::GetEnumNameByValue(Platform);;
		FString result = FString::Printf(TEXT("======== %s Non-Assets =========\n"),*PlatformName);
		uint32 BeginLength = result.Len();
		result += FString::Printf(TEXT("Add Non-Asset Number: %d\n"),ExternDiff.AddExternalFiles.Num());
		result += FString::Printf(TEXT("Modify Non-Asset Number: %d\n"),ExternDiff.ModifyExternalFiles.Num());
		result += FString::Printf(TEXT("Delete Non-Asset Number: %d\n"),ExternDiff.DeleteExternalFiles.Num());
		for(uint32 index=0;index<BeginLength;++index)
			result += TEXT("=");
		result+=TEXT("\n");
		return result;
	};
	
	FString result = TEXT("\n-----------------HotPatcher Patch Summary-----------------\n");
	result+= TEXT("=================\n");
	TMap<FString,uint32> AddModuleAssetNumMap;
	result += FString::Printf(TEXT("Add Asset Number: %d\n"),UFLibAssetManageHelperEx::ParserAssetDependenciesInfoNumber(DiffInfo.AssetDiffInfo.AddAssetDependInfo,AddModuleAssetNumMap));
	result += UFLibAssetManageHelperEx::ParserModuleAssetsNumMap(AddModuleAssetNumMap);
	TMap<FString,uint32> ModifyModuleAssetNumMap;
	result += FString::Printf(TEXT("Modify Asset Number: %d\n"),UFLibAssetManageHelperEx::ParserAssetDependenciesInfoNumber(DiffInfo.AssetDiffInfo.ModifyAssetDependInfo,ModifyModuleAssetNumMap));
	result += UFLibAssetManageHelperEx::ParserModuleAssetsNumMap(ModifyModuleAssetNumMap);
	TMap<FString,uint32> DeleteModuleAssetNumMap;
	result += FString::Printf(TEXT("Delete Asset Number: %d\n"),UFLibAssetManageHelperEx::ParserAssetDependenciesInfoNumber(DiffInfo.AssetDiffInfo.DeleteAssetDependInfo,DeleteModuleAssetNumMap));
	result += UFLibAssetManageHelperEx::ParserModuleAssetsNumMap(DeleteModuleAssetNumMap);
	TArray<ETargetPlatform> Platforms;
	DiffInfo.PlatformExternDiffInfo.GetKeys(Platforms);
	for(const auto& Platform:Platforms)
	{
		result += ExternFileSummary(Platform,*DiffInfo.PlatformExternDiffInfo.Find(Platform));
	}
	return result;
}


FString UFlibHotPatcherEditorHelper::MakePakShortName(const FHotPatcherVersion& InCurrentVersion, const FChunkInfo& InChunkInfo, const FString& InPlatform,const FString& InRegular)
{
	struct FResularOperator
	{
		FResularOperator(const FString& InName,TFunction<FString(void)> InOperator)
			:Name(InName),Do(InOperator){}
		FString Name;
		TFunction<FString(void)> Do;
	};
	
	TArray<FResularOperator> RegularOpList;
	RegularOpList.Emplace(TEXT("{VERSION}"),[&InCurrentVersion]()->FString{return InCurrentVersion.VersionId;});
	RegularOpList.Emplace(TEXT("{BASEVERSION}"),[&InCurrentVersion]()->FString{return InCurrentVersion.BaseVersionId;});
	RegularOpList.Emplace(TEXT("{PLATFORM}"),[&InPlatform]()->FString{return InPlatform;});
	RegularOpList.Emplace(TEXT("{CHUNKNAME}"),[&InChunkInfo,&InCurrentVersion]()->FString
	{
		if(!InCurrentVersion.VersionId.Equals(InChunkInfo.ChunkName))
			return InChunkInfo.ChunkName;
		else
			return TEXT("");
	});
	
	auto CustomPakNameRegular = [](const TArray<FResularOperator>& Operators,const FString& Regular)->FString
	{
		FString Result = Regular;
		for(auto& Operator:Operators)
		{
			Result = Result.Replace(*Operator.Name,*(Operator.Do()));
		}
		while(Result.Contains(TEXT("__")))
		{
			Result = Result.Replace(TEXT("__"),TEXT("_"));
		}
		return Result;
	};
	
	return CustomPakNameRegular(RegularOpList,InRegular);
}

bool UFlibHotPatcherEditorHelper::CheckSelectedAssetsCookStatus(const TArray<FString>& PlatformNames, const FAssetDependenciesInfo& SelectedAssets, FString& OutMsg)
{
	OutMsg.Empty();
	// 检查所修改的资源是否被Cook过
	for (const auto& PlatformName : PlatformNames)
	{
		TArray<FAssetDetail> ValidCookAssets;
		TArray<FAssetDetail> InvalidCookAssets;

		UFlibHotPatcherEditorHelper::CheckInvalidCookFilesByAssetDependenciesInfo(UKismetSystemLibrary::GetProjectDirectory(), PlatformName, SelectedAssets, ValidCookAssets, InvalidCookAssets);

		if (InvalidCookAssets.Num() > 0)
		{
			OutMsg.Append(FString::Printf(TEXT("%s UnCooked Assets:\n"), *PlatformName));

			for (const auto& Asset : InvalidCookAssets)
			{
				FString AssetLongPackageName;
				UFLibAssetManageHelperEx::ConvPackagePathToLongPackageName(Asset.mPackagePath, AssetLongPackageName);
				OutMsg.Append(FString::Printf(TEXT("\t%s\n"), *AssetLongPackageName));
			}
		}
	}

	return OutMsg.IsEmpty();
}

bool UFlibHotPatcherEditorHelper::CheckPatchRequire(const FPatchVersionDiff& InDiff,const TArray<FString>& PlatformNames,FString& OutMsg)
{
	bool Status = false;
	// 错误处理
	{
		FString GenErrorMsg;
		FAssetDependenciesInfo AllChangedAssetInfo = UFLibAssetManageHelperEx::CombineAssetDependencies(InDiff.AssetDiffInfo.AddAssetDependInfo, InDiff.AssetDiffInfo.ModifyAssetDependInfo);
		bool bSelectedCookStatus = CheckSelectedAssetsCookStatus(PlatformNames, AllChangedAssetInfo, GenErrorMsg);

		// 如果有错误信息 则输出后退出
		if (!bSelectedCookStatus)
		{
			OutMsg = GenErrorMsg;
			Status = false;
		}
		else
		{
			OutMsg = TEXT("");
			Status = true;
		}
	}
	return Status;
}

FString UFlibHotPatcherEditorHelper::Conv2IniPlatform(const FString& Platform)
{
	FString Result;
	static TMap<FString,FString> PlatformMaps;
	static bool bInit = false;
	if(!bInit)
	{
		ITargetPlatformManagerModule& TPM = GetTargetPlatformManagerRef();
		const TArray<ITargetPlatform*>& TargetPlatforms = TPM.GetTargetPlatforms();
		TArray<ITargetPlatform*> CookPlatforms; 
		for (ITargetPlatform *TargetPlatform : TargetPlatforms)
		{
			PlatformMaps.Add(TargetPlatform->PlatformName(),TargetPlatform->IniPlatformName());
		}
		bInit = true;
	}
	
	if(PlatformMaps.Contains(Platform))
	{
		Result = *PlatformMaps.Find(Platform);
	}
	return Result;
}

TArray<FString> UFlibHotPatcherEditorHelper::GetSupportPlatforms()
{
	TArray<FString> Result;
	ITargetPlatformManagerModule& TPM = GetTargetPlatformManagerRef();
	const TArray<ITargetPlatform*>& TargetPlatforms = TPM.GetTargetPlatforms();
	TArray<ITargetPlatform*> CookPlatforms; 
	for (ITargetPlatform *TargetPlatform : TargetPlatforms)
	{
		Result.AddUnique(TargetPlatform->PlatformName());
	}
	return Result;
}

#define ENCRYPT_CRYPTO_NAME TEXT("cryptokeys")

FString UFlibHotPatcherEditorHelper::GetEncryptSettingsCommandlineOptions(const FPakEncryptSettings& PakEncryptSettings,const FString& PlatformName)
{
	FString Result; 

	FString CryptoKey = UFlibPatchParserHelper::ReplaceMarkPath(PakEncryptSettings.CryptoKeys.FilePath);

	auto AppendCommandOptions = [&Result](bool bEncryptIndex,bool bbEncrypt,bool bSign)
	{
		if(bbEncrypt)
		{
			Result += TEXT("-encrypt ");
		}
		if(bEncryptIndex)
		{
			Result += TEXT("-encryptindex ");
		}
		if(bSign)
		{
			Result += TEXT("-sign ");
		}
	};
	
	FEncryptSetting EncryptSettings = UFlibPatchParserHelper::GetCryptoSettingByPakEncryptSettings(PakEncryptSettings);
	if(PakEncryptSettings.bUseDefaultCryptoIni)
	{
		FString SaveTo = FPaths::ConvertRelativePathToFull(FPaths::Combine(FPaths::ProjectSavedDir(),TEXT("HotPatcher"),TEXT("Crypto.json")));
		FPakEncryptionKeys PakEncryptionKeys = UFlibPatchParserHelper::GetCryptoByProjectSettings();
		UFlibPatchParserHelper::SerializePakEncryptionKeyToFile(PakEncryptionKeys,SaveTo);
		CryptoKey = SaveTo;
	}
	
	AppendCommandOptions(
		EncryptSettings.bEncryptIndex,
		(EncryptSettings.bEncryptIniFiles || EncryptSettings.bEncryptAllAssetFiles || EncryptSettings.bEncryptUAssetFiles),
		EncryptSettings.bSign
		);
	
	if(FPaths::FileExists(CryptoKey))
	{
		Result += FString::Printf(TEXT("-%s=\"%s\" "),ENCRYPT_CRYPTO_NAME,*CryptoKey);
	}
	
	Result += FString::Printf(TEXT("-projectdir=\"%s\" "),*FPaths::ConvertRelativePathToFull(FPaths::ProjectDir()));
	Result += FString::Printf(TEXT("-enginedir=\"%s\" "),*FPaths::ConvertRelativePathToFull(FPaths::EngineDir()));
	Result += FString::Printf(TEXT("-platform=%s"),*PlatformName);
	return Result;
}

ITargetPlatform* UFlibHotPatcherEditorHelper::GetPlatformByName(const FString& Name)
{
	static TMap<FString,ITargetPlatform*> PlatformNameMap;

	if(PlatformNameMap.Contains(Name))
		return *PlatformNameMap.Find(Name);
	
	ITargetPlatformManagerModule& TPM = GetTargetPlatformManagerRef();
	const TArray<ITargetPlatform*>& TargetPlatforms = TPM.GetTargetPlatforms();
	ITargetPlatform* result = NULL;
	for (ITargetPlatform *TargetPlatform : TargetPlatforms)
	{
		if (Name.Equals(TargetPlatform->PlatformName()))
		{
			result = TargetPlatform;
			PlatformNameMap.Add(Name,TargetPlatform);
			break;
		}
	}
	return result;
}
#include "Commandlets/AssetRegistryGenerator.h"

bool UFlibHotPatcherEditorHelper::GeneratorGlobalAssetRegistryData(ITargetPlatform* TargetPlatform, const TSet<FName>& CookedPackageNames, const TSet<FName>& IgnorePackageNames, bool
                                                             bGenerateStreamingInstallManifest)
{
	bool bresult = true;
#if GENERATE_ASSET_REGISTRY_DATA
	TUniquePtr<class FSandboxPlatformFile> TempSandboxFile = MakeUnique<FSandboxPlatformFile>(false);
	TUniquePtr<FAssetRegistryGenerator> RegistryGenerator = MakeUnique<FAssetRegistryGenerator>(TargetPlatform);
	RegistryGenerator->CleanManifestDirectories();
	RegistryGenerator->Initialize(TArray<FName>());
	RegistryGenerator->PreSave(CookedPackageNames);
	RegistryGenerator->BuildChunkManifest(CookedPackageNames, IgnorePackageNames, TempSandboxFile.Get(), bGenerateStreamingInstallManifest);
	bresult = RegistryGenerator->SaveManifests(TempSandboxFile.Get());
#endif
	return bresult;
}

FPatchVersionDiff UFlibHotPatcherEditorHelper::DiffPatchVersionWithPatchSetting(const FExportPatchSettings& PatchSetting, const FHotPatcherVersion& Base, const FHotPatcherVersion& New)
{
	FPatchVersionDiff VersionDiffInfo;
	FAssetDependenciesInfo BaseVersionAssetDependInfo = Base.AssetInfo;
	FAssetDependenciesInfo CurrentVersionAssetDependInfo = New.AssetInfo;

	UFlibPatchParserHelper::DiffVersionAssets(
		CurrentVersionAssetDependInfo,
		BaseVersionAssetDependInfo,
		VersionDiffInfo.AssetDiffInfo.AddAssetDependInfo,
		VersionDiffInfo.AssetDiffInfo.ModifyAssetDependInfo,
		VersionDiffInfo.AssetDiffInfo.DeleteAssetDependInfo
	);

	UFlibPatchParserHelper::DiffVersionAllPlatformExFiles(Base,New,VersionDiffInfo.PlatformExternDiffInfo);

	if(PatchSetting.GetIgnoreDeletionModulesAsset().Num())
	{
		for(const auto& ModuleName:PatchSetting.GetIgnoreDeletionModulesAsset())
		{
			VersionDiffInfo.AssetDiffInfo.DeleteAssetDependInfo.AssetsDependenciesMap.Remove(ModuleName);
		}
	}
	
	if(PatchSetting.IsRecursiveWidgetTree())
	{
		UFlibHotPatcherEditorHelper::AnalysisWidgetTree(VersionDiffInfo);
	}
	
	if(PatchSetting.IsForceSkipContent())
	{
		TArray<FString> AllSkipContents;
		AllSkipContents.Append(PatchSetting.GetForceSkipContentStrRules());
		AllSkipContents.Append(PatchSetting.GetForceSkipAssetsStr());
		UFlibPatchParserHelper::ExcludeContentForVersionDiff(VersionDiffInfo,AllSkipContents);
	}
	// clean deleted asset info in patch
	if(PatchSetting.IsIgnoreDeleatedAssetsInfo())
	{
		UE_LOG(LogHotPatcher,Display,TEXT("ignore deleted assets info in patch..."));
		VersionDiffInfo.AssetDiffInfo.DeleteAssetDependInfo.AssetsDependenciesMap.Empty();
		if(VersionDiffInfo.PlatformExternDiffInfo.Contains(ETargetPlatform::AllPlatforms))
		{
			VersionDiffInfo.PlatformExternDiffInfo.Find(ETargetPlatform::AllPlatforms)->DeleteExternalFiles.Empty();
		}
		for(const auto& Platform:PatchSetting.GetPakTargetPlatforms())
		{
			VersionDiffInfo.PlatformExternDiffInfo.Find(Platform)->DeleteExternalFiles.Empty();
		}
	}
	
	return VersionDiffInfo;
}

#include "BaseWidgetBlueprint.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "WidgetBlueprint.h"

void UFlibHotPatcherEditorHelper::AnalysisWidgetTree(FPatchVersionDiff& PakDiff,int32 flags)
{
	TArray<FAssetDetail> AnalysisAssets;
	if(flags & 0x1)
	{
		TArray<FAssetDetail> AddAssets;
		UFLibAssetManageHelperEx::GetAssetDetailsByAssetDependenciesInfo(PakDiff.AssetDiffInfo.AddAssetDependInfo,AddAssets);
		AnalysisAssets.Append(AddAssets);
		
	}
	if(flags & 0x2)
	{
		TArray<FAssetDetail> ModifyAssets;
		UFLibAssetManageHelperEx::GetAssetDetailsByAssetDependenciesInfo(PakDiff.AssetDiffInfo.ModifyAssetDependInfo,ModifyAssets);
		AnalysisAssets.Append(ModifyAssets);
	}
	TArray<EAssetRegistryDependencyTypeEx> AssetRegistryDepTypes {EAssetRegistryDependencyTypeEx::Hard};
	FString AssetType = TEXT("WidgetBlueprint");

	auto AssetsIsExist = [&PakDiff](const FAssetDetail& Asset)->bool
	{
		bool result = false;
		FString ModuleName = UFLibAssetManageHelperEx::GetAssetBelongModuleName(Asset.mPackagePath);
		FString LongPackageName;
		UFLibAssetManageHelperEx::ConvPackagePathToLongPackageName(Asset.mPackagePath,LongPackageName);
		if(PakDiff.AssetDiffInfo.ModifyAssetDependInfo.AssetsDependenciesMap.Find(ModuleName))
		{
			if(PakDiff.AssetDiffInfo.ModifyAssetDependInfo.AssetsDependenciesMap.Find(ModuleName)->AssetDependencyDetails.Find(LongPackageName))
			{
				result = true;
			}
		}
		return result;
	};
	TArray<EAssetRegistryDependencyType::Type> SearchType{EAssetRegistryDependencyType::Hard};
	
	for(const auto& OriginAsset:AnalysisAssets)
	{
		if(OriginAsset.mAssetType.Equals(AssetType))
		{
			// if asset is existed
			if(AssetsIsExist(OriginAsset))
			{
				continue;
			}
			TArray<FAssetDetail> CurrentAssetsRef;
			UFLibAssetManageHelperEx::GetAssetReferenceRecursively(OriginAsset, SearchType, TArray<FString>{AssetType}, CurrentAssetsRef);
			UE_LOG(LogHotPatcher,Display,TEXT("Reference %s Widgets:"),*OriginAsset.mPackagePath);
			// TArray<FAssetDetail> CurrentAssetDeps = UFlibPatchParserHelper::GetAllAssetDependencyDetails(OriginAsset,AssetRegistryDepTypes,AssetType);
			for(const auto& Asset:CurrentAssetsRef)
			{
				if(!Asset.mAssetType.Equals(AssetType))
				{
					continue;
				}
				
				UE_LOG(LogHotPatcher,Display,TEXT("Widget: %s"),*Asset.mPackagePath);
				FString ModuleName = UFLibAssetManageHelperEx::GetAssetBelongModuleName(Asset.mPackagePath);
				FString LongPackageName;
				UFLibAssetManageHelperEx::ConvPackagePathToLongPackageName(Asset.mPackagePath,LongPackageName);
				if(PakDiff.AssetDiffInfo.ModifyAssetDependInfo.AssetsDependenciesMap.Find(ModuleName))
				{
					PakDiff.AssetDiffInfo.ModifyAssetDependInfo.AssetsDependenciesMap.Find(ModuleName)->AssetDependencyDetails.Add(LongPackageName,Asset);
				}
				else
				{
					FAssetDependenciesDetail AssetModuleDetail;
					AssetModuleDetail.ModuleCategory = ModuleName;
					AssetModuleDetail.AssetDependencyDetails.Add(LongPackageName,Asset);
					PakDiff.AssetDiffInfo.ModifyAssetDependInfo.AssetsDependenciesMap.Add(ModuleName,AssetModuleDetail);
				}
			}
		}
	}
}
FChunkAssetDescribe UFlibHotPatcherEditorHelper::DiffChunkWithPatchSetting(
	const FExportPatchSettings& PatchSetting,
	const FChunkInfo& CurrentVersionChunk,
	const FChunkInfo& TotalChunk,
	TMap<FString, FAssetDependenciesInfo>& ScanedCaches
)
{
	FHotPatcherVersion TotalChunkVersion = UFlibPatchParserHelper::ExportReleaseVersionInfoByChunk(
		TEXT(""),
		TEXT(""),
		TEXT(""),
		TotalChunk,
		ScanedCaches,
		PatchSetting.IsIncludeHasRefAssetsOnly(),
		TotalChunk.bAnalysisFilterDependencies
	);

	return UFlibHotPatcherEditorHelper::DiffChunkByBaseVersionWithPatchSetting(PatchSetting, CurrentVersionChunk ,TotalChunk, TotalChunkVersion,ScanedCaches);
}

FChunkAssetDescribe UFlibHotPatcherEditorHelper::DiffChunkByBaseVersionWithPatchSetting(
	const FExportPatchSettings& PatchSetting,
	const FChunkInfo& CurrentVersionChunk,
	const FChunkInfo& TotalChunk,
	const FHotPatcherVersion& BaseVersion,
	TMap<FString, FAssetDependenciesInfo>& ScanedCaches
)
{
	FChunkAssetDescribe result;
	FHotPatcherVersion CurrentVersion = UFlibPatchParserHelper::ExportReleaseVersionInfoByChunk(
		TEXT(""),
		TEXT(""),
		TEXT(""),
		CurrentVersionChunk,
		ScanedCaches,
		PatchSetting.IsIncludeHasRefAssetsOnly(),
		CurrentVersionChunk.bAnalysisFilterDependencies
	);
	FPatchVersionDiff ChunkDiffInfo = UFlibHotPatcherEditorHelper::DiffPatchVersionWithPatchSetting(PatchSetting, BaseVersion, CurrentVersion);
	
	result.Assets = UFLibAssetManageHelperEx::CombineAssetDependencies(ChunkDiffInfo.AssetDiffInfo.AddAssetDependInfo, ChunkDiffInfo.AssetDiffInfo.ModifyAssetDependInfo);

	TArray<ETargetPlatform> Platforms;
	ChunkDiffInfo.PlatformExternDiffInfo.GetKeys(Platforms);
	for(auto Platform:Platforms)
	{
		FPlatformExternFiles PlatformFiles;
		PlatformFiles.Platform = Platform;
		PlatformFiles.ExternFiles = ChunkDiffInfo.PlatformExternDiffInfo.Find(Platform)->AddExternalFiles;
		PlatformFiles.ExternFiles.Append(ChunkDiffInfo.PlatformExternDiffInfo.Find(Platform)->ModifyExternalFiles);
		result.AllPlatformExFiles.Add(Platform,PlatformFiles);	
	}
	
	result.InternalFiles.bIncludeAssetRegistry = CurrentVersionChunk.InternalFiles.bIncludeAssetRegistry != TotalChunk.InternalFiles.bIncludeAssetRegistry;
	result.InternalFiles.bIncludeGlobalShaderCache = CurrentVersionChunk.InternalFiles.bIncludeGlobalShaderCache != TotalChunk.InternalFiles.bIncludeGlobalShaderCache;
	result.InternalFiles.bIncludeShaderBytecode = CurrentVersionChunk.InternalFiles.bIncludeShaderBytecode != TotalChunk.InternalFiles.bIncludeShaderBytecode;
	result.InternalFiles.bIncludeEngineIni = CurrentVersionChunk.InternalFiles.bIncludeEngineIni != TotalChunk.InternalFiles.bIncludeEngineIni;
	result.InternalFiles.bIncludePluginIni = CurrentVersionChunk.InternalFiles.bIncludePluginIni != TotalChunk.InternalFiles.bIncludePluginIni;
	result.InternalFiles.bIncludeProjectIni = CurrentVersionChunk.InternalFiles.bIncludeProjectIni != TotalChunk.InternalFiles.bIncludeProjectIni;

	return result;
}

bool UFlibHotPatcherEditorHelper::SerializeAssetRegistryByDetails(const FString& PlatformName,
	const TArray<FAssetDetail>& AssetDetails, const FString& SavePath)
{
	TArray<FString> PackagePaths;

	for(const auto& Detail:AssetDetails)
	{
		PackagePaths.AddUnique(Detail.mPackagePath);
	}
	return UFlibHotPatcherEditorHelper::SerializeAssetRegistry(PlatformName,PackagePaths,SavePath);
}

bool UFlibHotPatcherEditorHelper::SerializeAssetRegistry(const FString& PlatformName,
                                                         const TArray<FString>& PackagePaths, const FString& SavePath)
{
	ITargetPlatform* TargetPlatform =  UFlibHotPatcherEditorHelper::GetPlatformByName(PlatformName);
	FAssetRegistryState State;
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
					
	FAssetRegistrySerializationOptions SaveOptions;
	AssetRegistry.InitializeSerializationOptions(SaveOptions, TargetPlatform->IniPlatformName());

	SaveOptions.bSerializeAssetRegistry = true;
					
	AssetRegistry.Tick(-1.0f);
	AssetRegistry.InitializeTemporaryAssetRegistryState(State, SaveOptions, true);
	for(const auto& AssetPackagePath:PackagePaths)
	{
		FAssetData* AssetData = new FAssetData();
		if(UFLibAssetManageHelperEx::GetSingleAssetsData(AssetPackagePath,*AssetData))
		{
			State.AddAssetData(AssetData);
		}
	}
	// Create runtime registry data
	FArrayWriter SerializedAssetRegistry;
	SerializedAssetRegistry.SetFilterEditorOnly(true);

#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 26
	bool bStateSave = State.Save(SerializedAssetRegistry, SaveOptions);
#else
	bool bStateSave = State.Serialize(SerializedAssetRegistry, SaveOptions);
#endif
	bool result = false;
	// Save the generated registry
	if(bStateSave && FFileHelper::SaveArrayToFile(SerializedAssetRegistry, *SavePath))
	{
		result = true;
		UE_LOG(LogHotPatcher,Log,TEXT("Serialize %s AssetRegistry"),*SavePath);
	}
	return result;
}


FHotPatcherVersion UFlibHotPatcherEditorHelper::MakeNewRelease(const FHotPatcherVersion& InBaseVersion, const FHotPatcherVersion& InCurrentVersion, FExportPatchSettings* InPatchSettings)
{
	FHotPatcherVersion BaseVersion = InBaseVersion;
	
	FPatchVersionDiff DiffInfo = UFlibHotPatcherEditorHelper::DiffPatchVersionWithPatchSetting(*InPatchSettings,BaseVersion, InCurrentVersion);
	return UFlibHotPatcherEditorHelper::MakeNewReleaseByDiff(InBaseVersion,DiffInfo, InPatchSettings);
}

FHotPatcherVersion UFlibHotPatcherEditorHelper::MakeNewReleaseByDiff(const FHotPatcherVersion& InBaseVersion,
	const FPatchVersionDiff& InDiff, FExportPatchSettings* InPatchSettings)
{
	FHotPatcherVersion BaseVersion = InBaseVersion;
	FHotPatcherVersion NewRelease;

	NewRelease.BaseVersionId = InBaseVersion.VersionId;
	NewRelease.Date = FDateTime::UtcNow().ToString();
	NewRelease.VersionId = InPatchSettings->VersionId;
	
	FAssetDependenciesInfo& BaseAssetInfoRef = BaseVersion.AssetInfo;
	// TMap<FString, FExternFileInfo>& BaseExternalFilesRef = BaseVersion.ExternalFiles;
	TMap<ETargetPlatform,FPlatformExternAssets>& BasePlatformAssetsRef = BaseVersion.PlatformAssets;

	// Modify Asset
	auto DeleteOldAssetLambda = [&BaseAssetInfoRef](const FAssetDependenciesInfo& InAssetDependenciesInfo)
	{
		for (const auto& AssetsModulePair : InAssetDependenciesInfo.AssetsDependenciesMap)
		{
			FAssetDependenciesDetail* NewReleaseModuleAssets = BaseAssetInfoRef.AssetsDependenciesMap.Find(AssetsModulePair.Key);

			for (const auto& NeedDeleteAsset : AssetsModulePair.Value.AssetDependencyDetails)
			{
				if (NewReleaseModuleAssets && NewReleaseModuleAssets->AssetDependencyDetails.Contains(NeedDeleteAsset.Key))
				{
					NewReleaseModuleAssets->AssetDependencyDetails.Remove(NeedDeleteAsset.Key);
				}
			}
		}
	};
	
	DeleteOldAssetLambda(InDiff.AssetDiffInfo.ModifyAssetDependInfo);
	if(InPatchSettings && !InPatchSettings->IsSaveDeletedAssetsToNewReleaseJson())
	{
		DeleteOldAssetLambda(InDiff.AssetDiffInfo.DeleteAssetDependInfo);
	}

	// Add Asset
	BaseAssetInfoRef = UFLibAssetManageHelperEx::CombineAssetDependencies(BaseAssetInfoRef, InDiff.AssetDiffInfo.AddAssetDependInfo);
	// modify Asset
	BaseAssetInfoRef = UFLibAssetManageHelperEx::CombineAssetDependencies(BaseAssetInfoRef, InDiff.AssetDiffInfo.ModifyAssetDependInfo);
	NewRelease.AssetInfo = BaseAssetInfoRef;

	// // external files
	// auto RemoveOldExternalFilesLambda = [&BaseExternalFilesRef](const TArray<FExternFileInfo>& InFiles)
	// {
	// 	for (const auto& File : InFiles)
	// 	{
	// 		if (BaseExternalFilesRef.Contains(File.FilePath.FilePath))
	// 		{
	// 			BaseExternalFilesRef.Remove(File.FilePath.FilePath);
	// 		}
	// 	}
	// };

	TArray<ETargetPlatform> DiffPlatforms;
	InDiff.PlatformExternDiffInfo.GetKeys(DiffPlatforms);

	for(auto Platform:DiffPlatforms)
	{
		FPlatformExternAssets AddPlatformFiles;
		AddPlatformFiles.TargetPlatform = Platform;
		AddPlatformFiles.AddExternFileToPak = InDiff.PlatformExternDiffInfo[Platform].AddExternalFiles;
		AddPlatformFiles.AddExternFileToPak.Append(InDiff.PlatformExternDiffInfo[Platform].ModifyExternalFiles);
		if(BasePlatformAssetsRef.Contains(Platform))
		{
			for(const auto& File:AddPlatformFiles.AddExternFileToPak)
			{
				if(BasePlatformAssetsRef[Platform].AddExternFileToPak.Contains(File))
				{
					BasePlatformAssetsRef[Platform].AddExternFileToPak.Remove(File);
				}
				BasePlatformAssetsRef[Platform].AddExternFileToPak.Add(File);
			}
		}else
		{
			BasePlatformAssetsRef.Add(Platform,AddPlatformFiles);
		}
	}
	// RemoveOldExternalFilesLambda(DiffInfo.ExternDiffInfo.ModifyExternalFiles);
	// DeleteOldExternalFilesLambda(DiffInfo.DeleteExternalFiles);

	NewRelease.PlatformAssets = BasePlatformAssetsRef;
	return NewRelease;
}


FString UFlibHotPatcherEditorHelper::GetPakCommandExtersion(const FString& InCommand)
{
	auto GetPakCommandFileExtensionLambda = [](const FString& Command)->FString
	{
		FString result;
		int32 DotPos = Command.Find(TEXT("."), ESearchCase::CaseSensitive, ESearchDir::FromEnd);
		if (DotPos != INDEX_NONE)
		{
			result =  Command.Mid(DotPos + 1);
			int32 FirstDoubleQuotesPos = -1;
			if(result.FindChar('"',FirstDoubleQuotesPos))
			{
				result.RemoveAt(FirstDoubleQuotesPos,result.Len()-FirstDoubleQuotesPos);
			}
		}
		return result;
	};
	return GetPakCommandFileExtensionLambda(InCommand);
}

TArray<FString> UFlibHotPatcherEditorHelper::GetExtensionsToNotUsePluginCompressionByGConfig()
{
	TArray<FString> IgnoreCompressFormats;
	GConfig->GetArray(TEXT("Pak"),TEXT("ExtensionsToNotUsePluginCompression"),IgnoreCompressFormats,GEngineIni);
	return IgnoreCompressFormats;
}

void UFlibHotPatcherEditorHelper::AppendPakCommandOptions(TArray<FString>& OriginCommands,
	const TArray<FString>& Options, bool bAppendAllMatch, const TArray<FString>& AppendFileExtersions,
	const TArray<FString>& IgnoreFormats, const TArray<FString>& InIgnoreOptions)
{
	for(auto& Command:OriginCommands)
	{
		FString PakOptionsStr;
		for (const auto& Param : Options)
		{
			FString FileExtension = UFlibHotPatcherEditorHelper::GetPakCommandExtersion(Command);
			if(IgnoreFormats.Contains(FileExtension) && InIgnoreOptions.Contains(Param))
			{
				continue;
			}
							
			FString AppendOptionStr = TEXT("");
			if(bAppendAllMatch || AppendFileExtersions.Contains(FileExtension))
			{
				AppendOptionStr += TEXT(" ") + Param;
			}
							
			PakOptionsStr += AppendOptionStr;
		}
		Command = FString::Printf(TEXT("%s%s"),*Command,*PakOptionsStr);
	}
}

FProjectPackageAssetCollection UFlibHotPatcherEditorHelper::ImportProjectSettingsPackages()
{
	FProjectPackageAssetCollection result;
	TArray<FDirectoryPath>& DirectoryPaths = result.DirectoryPaths;
	TArray<FSoftObjectPath>& SoftObjectPaths = result.NeedCookPackages;

	
	auto AddSoftObjectPath = [&](const FString& LongPackageName)
	{
		FString LongPackagePath = UFLibAssetManageHelperEx::LongPackageNameToPackagePath(LongPackageName);
		bool bSuccessed = false;
		if (!LongPackagePath.IsEmpty() && UAssetManager::Get().VerifyCanCookPackage(FName(*LongPackageName),false)
			&& !FPackageName::IsScriptPackage(LongPackagePath) && !FPackageName::IsMemoryPackage(LongPackagePath))
		{
			
			FSoftObjectPath CurrentObject(LongPackagePath);
			if(FPackageName::DoesPackageExist(LongPackagePath) && CurrentObject.IsValid()) 
			{
				// UE_LOG(LogHotPatcherEditorHelper,Display,TEXT("Import Project Setting Package: %s"),*LongPackagePath);
				SoftObjectPaths.AddUnique(CurrentObject);
				bSuccessed = true;
			}
		}

		if(!bSuccessed)
		{
			// UE_LOG(LogHotPatcherEditorHelper,Warning,TEXT("Import Project Setting Package: %s is inavlid!"),*LongPackagePath);
		}
	};
	
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry* AssetRegistry = &AssetRegistryModule.Get();
	
	const UProjectPackagingSettings* const PackagingSettings = GetDefault<UProjectPackagingSettings>();
	
	{
		// allow the game to fill out the asset registry, as well as get a list of objects to always cook
		TArray<FString> FilesInPathStrings;
		FGameDelegates::Get().GetCookModificationDelegate().ExecuteIfBound(FilesInPathStrings);
		for(const auto& BuildFilename:FilesInPathStrings)
		{
			FString OutPackageName;
			if (FPackageName::TryConvertFilenameToLongPackageName(BuildFilename, OutPackageName))
			{
				AddSoftObjectPath(OutPackageName);
			}
		}
	}
	
	// in Asset Manager / PrimaryAssetLabel
	{
		TArray<FName> PackageToCook;
		TArray<FName> PackageToNeverCook;
		UAssetManager::Get().ModifyCook(PackageToCook,PackageToNeverCook);
		for(const auto& Package:PackageToCook)
		{
			AddSoftObjectPath(Package.ToString());
		}
		for(const auto& NeverCookPackage:PackageToNeverCook)
		{
			result.NeverCookPackages.Add(NeverCookPackage.ToString());
		}
	}

	// DirectoriesToAlwaysCook
	DirectoryPaths.Append(PackagingSettings->DirectoriesToAlwaysCook);

	// AlwaysCookMaps
	{
		TArray<FString> MapList;
		// Add the default map section
		GEditor->LoadMapListFromIni(TEXT("AlwaysCookMaps"), MapList);

		for (int32 MapIdx = 0; MapIdx < MapList.Num(); MapIdx++)
		{
			FName PackageName = FName(*FPackageName::FilenameToLongPackageName(MapList[MapIdx]));
			AddSoftObjectPath(PackageName.ToString());
		}
	}

	// MapsToCook
	for (const FFilePath& MapToCook : PackagingSettings->MapsToCook)
	{
		FString File = MapToCook.FilePath;
		FName PackageName = FName(*FPackageName::FilenameToLongPackageName(File));
		AddSoftObjectPath(PackageName.ToString());
	}

	// Loading default map ini section AllMaps
	{
		TArray<FString> AllMapsSection;
		GEditor->LoadMapListFromIni(TEXT("AllMaps"), AllMapsSection);
		for (const FString& MapName : AllMapsSection)
		{
			AddSoftObjectPath(MapName);
		}
	}

	// all uasset and umap
	if(!SoftObjectPaths.Num() && !DirectoryPaths.Num())
	{
		TArray<FString> Tokens;
		Tokens.Empty(2);
		Tokens.Add(FString("*") + FPackageName::GetAssetPackageExtension());
		Tokens.Add(FString("*") + FPackageName::GetMapPackageExtension());

		uint8 PackageFilter = NORMALIZE_DefaultFlags | NORMALIZE_ExcludeEnginePackages | NORMALIZE_ExcludeLocalizedPackages;
		bool bMapsOnly = false;
		if (bMapsOnly)
		{
			PackageFilter |= NORMALIZE_ExcludeContentPackages;
		}
		bool bNoDev = false;
		if (bNoDev)
		{
			PackageFilter |= NORMALIZE_ExcludeDeveloperPackages;
		}

		// assume the first token is the map wildcard/pathname
		TArray<FString> Unused;
		for (int32 TokenIndex = 0; TokenIndex < Tokens.Num(); TokenIndex++)
		{
			TArray<FString> TokenFiles;
			if (!NormalizePackageNames(Unused, TokenFiles, Tokens[TokenIndex], PackageFilter))
			{
				UE_LOG(LogHotPatcherEditorHelper, Display, TEXT("No packages found for parameter %i: '%s'"), TokenIndex, *Tokens[TokenIndex]);
				continue;
			}

			for (int32 TokenFileIndex = 0; TokenFileIndex < TokenFiles.Num(); ++TokenFileIndex)
			{
				FName PackageName = FName(*FPackageName::FilenameToLongPackageName(TokenFiles[TokenFileIndex]));
				AddSoftObjectPath(PackageName.ToString());
			}
		}
	}
	
	// ===============================================

	{
		TSet<FName> StartupPackages;
		TSet<FName> StartupSoftObjectPackages;

		for (TObjectIterator<UPackage> It; It; ++It)
		{
			if ((*It) != GetTransientPackage())
			{
				if(!It->GetFName().ToString().StartsWith(TEXT("/Script")))
				{
					StartupPackages.Add(It->GetFName());
				}
			}
		}
		
		// Get the list of soft references, for both empty package and all startup packages
		GRedirectCollector.ProcessSoftObjectPathPackageList(NAME_None, false, StartupSoftObjectPackages);

		for (const FName& StartupPackage : StartupPackages)
		{
			GRedirectCollector.ProcessSoftObjectPathPackageList(StartupPackage, false, StartupSoftObjectPackages);
		}

		// Add string asset packages after collecting files, to avoid accidentally activating the behavior to cook all maps if none are specified
		for (FName SoftObjectPackage : StartupSoftObjectPackages)
		{
			AddSoftObjectPath(SoftObjectPackage.ToString());
		}
	}
	// Find all the localized packages and map them back to their source package
	{
		TArray<FString> AllCulturesToCook;
		for (const FString& CultureName : PackagingSettings->CulturesToStage)
		{
			const TArray<FString> PrioritizedCultureNames = FInternationalization::Get().GetPrioritizedCultureNames(CultureName);
			for (const FString& PrioritizedCultureName : PrioritizedCultureNames)
			{
				AllCulturesToCook.AddUnique(PrioritizedCultureName);
			}
		}
		AllCulturesToCook.Sort();

		TArray<FString> RootPaths;
		FPackageName::QueryRootContentPaths(RootPaths);

		FARFilter Filter;
		Filter.bRecursivePaths = true;
		Filter.bIncludeOnlyOnDiskAssets = false;
		Filter.PackagePaths.Reserve(AllCulturesToCook.Num() * RootPaths.Num());
		for (const FString& RootPath : RootPaths)
		{
			for (const FString& CultureName : AllCulturesToCook)
			{
				FString LocalizedPackagePath = RootPath / TEXT("L10N") / CultureName;
				Filter.PackagePaths.Add(*LocalizedPackagePath);
			}
		}

		TArray<FAssetData> AssetDataForCultures;
		AssetRegistry->GetAssets(Filter, AssetDataForCultures);

		for (const FAssetData& AssetData : AssetDataForCultures)
		{
			const FName LocalizedPackageName = AssetData.PackageName;
			const FName SourcePackageName = *FPackageName::GetSourcePackagePath(LocalizedPackageName.ToString());
			AddSoftObjectPath(LocalizedPackageName.ToString());
		}
	}
	const UGameMapsSettings* const GameMapsSettings = GetDefault<UGameMapsSettings>();
	{
		if(GameMapsSettings->GameInstanceClass.IsAsset())
		{
			AddSoftObjectPath(GameMapsSettings->GameInstanceClass.GetAssetPathString());
		}
	
		FSoftObjectPath GameDefaultMap{
			(GameMapsSettings->GetGameDefaultMap())
		};
		if(GameDefaultMap.IsAsset())
		{
			AddSoftObjectPath(GameDefaultMap.GetAssetPathString());
		}
	
		FSoftObjectPath DefaultGameMode{
			(GameMapsSettings->GetGlobalDefaultGameMode())
		};
		if(DefaultGameMode.IsAsset())
		{
			AddSoftObjectPath(DefaultGameMode.GetAssetPathString());
		}
	
		if(GameMapsSettings->TransitionMap.IsAsset())
		{
			AddSoftObjectPath(GameMapsSettings->TransitionMap.GetAssetPathString());
		}
	}
	auto CreateDirectory = [](const FString& Path)
	{
		FDirectoryPath DirectoryPath;
		DirectoryPath.Path = Path;
		return DirectoryPath;
	};

	

	// DirectoryPaths.AddUnique(CreateDirectory("/Game/UI"));
	// DirectoryPaths.AddUnique(CreateDirectory("/Game/Widget"));
	// DirectoryPaths.AddUnique(CreateDirectory("/Game/Widgets"));
	// DirectoryPaths.AddUnique(CreateDirectory("/Engine/MobileResources"));
	{
		TArray<FString> UIContentPaths;
		TSet <FName> ContentDirectoryAssets; 
		if (GConfig->GetArray(TEXT("UI"), TEXT("ContentDirectories"), UIContentPaths, GEditorIni) > 0)
		{
			for (int32 DirIdx = 0; DirIdx < UIContentPaths.Num(); DirIdx++)
			{
				DirectoryPaths.Add(CreateDirectory(UIContentPaths[DirIdx]));
			}
		}
	}

	{
		FConfigFile InputIni;
		FString InterfaceFile;
		FConfigCacheIni::LoadLocalIniFile(InputIni, TEXT("Input"), true);
		if (InputIni.GetString(TEXT("/Script/Engine.InputSettings"), TEXT("DefaultTouchInterface"), InterfaceFile))
		{
			if (InterfaceFile != TEXT("None") && InterfaceFile != TEXT(""))
			{
				AddSoftObjectPath(InterfaceFile);
			}
		}
	}

	{
		TArray<FString> AllCulturesToCook = PackagingSettings->CulturesToStage;;
		
		TArray<FString> RootPaths;
		FPackageName::QueryRootContentPaths(RootPaths);

		FARFilter Filter;
		Filter.bRecursivePaths = true;
		Filter.bIncludeOnlyOnDiskAssets = false;
		Filter.PackagePaths.Reserve(AllCulturesToCook.Num() * RootPaths.Num());
		for (const FString& RootPath : RootPaths)
		{
			for (const FString& CultureName : AllCulturesToCook)
			{
				FString LocalizedPackagePath = RootPath / TEXT("L10N") / CultureName;
				Filter.PackagePaths.Add(*LocalizedPackagePath);
			}
		}

		
		TArray<FAssetData> AssetDataForCultures;
		AssetRegistry->GetAssets(Filter, AssetDataForCultures);

		for (const FAssetData& AssetData : AssetDataForCultures)
		{
			// const FName LocalizedPackageName = AssetData.PackageName;
			// const FName SourcePackageName = *FPackageName::GetSourcePackagePath(LocalizedPackageName.ToString());

			AddSoftObjectPath(AssetData.PackageName.ToString());
		}
	}

	// GetUnsolicitedPackages
	{
		for (TObjectIterator<UPackage> It; It; ++It)
		{
			UPackage* Package = *It;

			if (Package->GetOuter() == nullptr)
			{
				AddSoftObjectPath(Package->GetName());
			}
		}
	}
	
	// never cook packages
	result.NeverCookPaths.Append(PackagingSettings->DirectoriesToNeverCook);

	return result;
}

