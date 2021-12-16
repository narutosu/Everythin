// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

//project header
#include "FChunkInfo.h"
#include "FPakFileInfo.h"
#include "FReplaceText.h"
#include "AssetManager/FAssetDependenciesInfo.h"
#include "FHotPatcherVersion.h"
#include "FPatchVersionDiff.h"
#include "FlibPakHelper.h"
#include "FExternDirectoryInfo.h"
#include "FExternDirectoryInfo.h"
#include "FHotPatcherAssetDependency.h"
#include "FCookerConfig.h"
#include "FPlatformExternFiles.h"

// cpp standard
#include <typeinfo>
#include <cctype>
#include <algorithm>
#include <string>

// engine header
#include "CoreMinimal.h"
#include "Resources/Version.h"
#include "JsonObjectConverter.h"
#include "Misc/CommandLine.h"
#include "FPlatformExternAssets.h"
#include "AssetRegistryState.h"
#include "Containers/UnrealString.h"
#include "CreatePatch/FExportPatchSettings.h"
#include "Templates/SharedPointer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "FlibPatchParserHelper.generated.h"

#if  ENGINE_MAJOR_VERSION <=4 && ENGINE_MINOR_VERSION < 25
#define FProperty UProperty
#endif

struct FExportPatchSettings;
/**
 * 
 */
UCLASS()
class HOTPATCHERRUNTIME_API UFlibPatchParserHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:

	UFUNCTION(BlueprintCallable, Category = "HotPatcher|Flib")
		static TArray<FString> GetAvailableMaps(FString GameName, bool IncludeEngineMaps,bool IncludePluginMaps, bool Sorted);
	UFUNCTION(BlueprintCallable, Category = "HotPatcher|Flib")
		static FString GetProjectName();
	UFUNCTION(BlueprintCallable)
	static FString GetProjectFilePath();
	
	static FHotPatcherVersion ExportReleaseVersionInfo(
        const FString& InVersionId,
        const FString& InBaseVersion,
        const FString& InDate,
        const TArray<FString>& InIncludeFilter,
        const TArray<FString>& InIgnoreFilter,
        const TArray<EAssetRegistryDependencyTypeEx>& AssetRegistryDependencyTypes,
        const TArray<FPatcherSpecifyAsset>& InIncludeSpecifyAsset,
        // const TArray<FExternAssetFileInfo>& InAllExternFiles,
        const TArray<FPlatformExternAssets>& AddToPlatformExFiles,
        TMap<FString, FAssetDependenciesInfo>& ScanedCaches,
        bool InIncludeHasRefAssetsOnly = false,
        bool bInAnalysisFilterDepend = true
    );
	static FHotPatcherVersion ExportReleaseVersionInfoByChunk(
		const FString& InVersionId,
		const FString& InBaseVersion,
		const FString& InDate,
		const FChunkInfo& InChunkInfo,
		TMap<FString, FAssetDependenciesInfo>& ScanedCaches,
		bool InIncludeHasRefAssetsOnly = false,
		bool bInAnalysisFilterDepend = true
	);


	UFUNCTION(BlueprintCallable, Category = "HotPatcher|Flib")
		static bool DiffVersionAssets(const FAssetDependenciesInfo& InNewVersion, 
								const FAssetDependenciesInfo& InBaseVersion,
								FAssetDependenciesInfo& OutAddAsset,
								FAssetDependenciesInfo& OutModifyAsset,
								FAssetDependenciesInfo& OutDeleteAsset
		);

	UFUNCTION()
	static bool DiffVersionAllPlatformExFiles(
        const FHotPatcherVersion& InBaseVersion,
        const FHotPatcherVersion& InNewVersion,
		TMap<ETargetPlatform,FPatchVersionExternDiff>& OutDiff        
    );
	UFUNCTION()
	static FPlatformExternFiles GetAllExFilesByPlatform(const FPlatformExternAssets& InPlatformConf,bool InGeneratedHash=true);
	UFUNCTION(BlueprintCallable, Category = "HotPatcher|Flib")
		static bool GetPakFileInfo(const FString& InFile,FPakFileInfo& OutFileInfo);

	// Cooked/PLATFORM_NAME/Engine/GlobalShaderCache-*.bin
	UFUNCTION(BlueprintCallable, Category = "HotPatcher|Flib")
		static TArray<FString> GetCookedGlobalShaderCacheFiles(const FString& InProjectDir,const FString& InPlatformName);
	// Cooked/PLATFORN_NAME/PROJECT_NAME/AssetRegistry.bin
	UFUNCTION(BlueprintCallable, Category = "HotPatcher|Flib")
		static bool GetCookedAssetRegistryFiles(const FString& InProjectAbsDir, const FString& InProjectName, const FString& InPlatformName,FString& OutFiles);
	// Cooked/PLATFORN_NAME/PROJECT_NAME/Content/ShaderArchive-*.ushaderbytecode
	UFUNCTION(BlueprintCallable, Category = "HotPatcher|Flib")
		static bool GetCookedShaderBytecodeFiles(const FString& InProjectAbsDir, const FString& InProjectName, const FString& InPlatformName,bool InGalobalBytecode,bool InProjectBytecode, TArray<FString>& OutFiles);

	// UFUNCTION(BlueprintCallable, Category = "HotPatcher|Flib")
		static bool ConvIniFilesToPakCommands(
			const FString& InEngineAbsDir, 
			const FString& InProjectAbsDir, 
			const FString& InProjectName, 
			// const TArray<FString>& InPakOptions, 
			const TArray<FString>& InIniFiles, 
			TArray<FString>& OutCommands, 
			TFunction<void(const FPakCommand&)> InReceiveCommand = [](const FPakCommand&) {});

	// UFUNCTION(BlueprintCallable, Category = "HotPatcher|Flib")
		static bool ConvNotAssetFileToPakCommand(
			const FString& InProjectDir,
			const FString& InPlatformName, 
			// const TArray<FString>& InPakOptions,
			const FString& InCookedFile,
			FString& OutCommand,
			TFunction<void(const FPakCommand&)> InReceiveCommand = [](const FPakCommand&) {});
	static bool ConvNotAssetFileToExFile(const FString& InProjectDir, const FString& InPlatformName, const FString& InCookedFile, FExternFileInfo& OutExFile);
	UFUNCTION(BlueprintCallable, Category = "HotPatcher|Flib")
	static FString HashStringWithSHA1(const FString &InString);

	UFUNCTION(BlueprintCallable, Category = "HotPatcher|Flib")
		static TArray<FString> GetIniConfigs(const FString& InSearchDir, const FString& InPlatformName);
	// return abslute path
	UFUNCTION(BlueprintCallable, Category = "HotPatcher|Flib")
	static TArray<FString> GetProjectIniFiles(const FString& InProjectDir,const FString& InPlatformName);

	UFUNCTION(BlueprintCallable, Category = "HotPatcher|Flib")
	static TArray<FString> GetEngineConfigs(const FString& InPlatformName);

	UFUNCTION(BlueprintCallable, Category = "HotPatcher|Flib")
	static TArray<FString> GetEnabledPluginConfigs(const FString& InPlatformName);


	static TArray<FExternFileInfo> ParserExDirectoryAsExFiles(const TArray<FExternDirectoryInfo>& InExternDirectorys);
	static TArray<FAssetDetail> ParserExFilesInfoAsAssetDetailInfo(const TArray<FExternFileInfo>& InExFiles);

	// get Engine / Project / Plugin ini files
	static TArray<FString> GetIniFilesByPakInternalInfo(const FPakInternalInfo& InPakInternalInfo,const FString& PlatformName);
	// get AssetRegistry.bin / GlobalShaderCache / ShaderBytecode
	static TArray<FString> GetCookedFilesByPakInternalInfo(
		const FPakInternalInfo& InPakInternalInfo, 
		const FString& PlatformName);

	static TArray<FExternFileInfo> GetInternalFilesAsExFiles(const FPakInternalInfo& InPakInternalInfo, const FString& InPlatformName);
	static TArray<FString> GetPakCommandsFromInternalInfo(
		const FPakInternalInfo& InPakInternalInfo, 
		const FString& PlatformName, 
		// const TArray<FString>& InPakOptions, 
		TFunction<void(const FPakCommand&)> InReceiveCommand=[](const FPakCommand&) {});
	
	static FChunkInfo CombineChunkInfo(const FChunkInfo& R, const FChunkInfo& L);
	static FChunkInfo CombineChunkInfos(const TArray<FChunkInfo>& Chunks);

	static TArray<FString> GetDirectoryPaths(const TArray<FDirectoryPath>& InDirectoryPath);
	
	// static TArray<FExternFileInfo> GetExternFilesFromChunk(const FChunkInfo& InChunk, TArray<ETargetPlatform> InTargetPlatforms, bool bCalcHash = false);
	TMap<ETargetPlatform,FPlatformExternFiles> GetAllPlatformExternFilesFromChunk(const FChunkInfo& InChunk, bool bCalcHash);

	static FChunkAssetDescribe CollectFChunkAssetsDescribeByChunk(
		const FPatchVersionDiff& DiffInfo,
		const FChunkInfo& Chunk, TArray<ETargetPlatform> Platforms,
		TMap<FString, FAssetDependenciesInfo>& ScanedCaches
	);

	static TArray<FString> CollectPakCommandsStringsByChunk(
		const FPatchVersionDiff& DiffInfo,
		const FChunkInfo& Chunk,
		const FString& PlatformName,
		// const TArray<FString>& PakOptions,
		TMap<FString, FAssetDependenciesInfo>& ScanedCaches,
		const FExportPatchSettings* PatcheSettings = nullptr
	);

	static TArray<FPakCommand> CollectPakCommandByChunk(
		const FPatchVersionDiff& DiffInfo,
		const FChunkInfo& Chunk,
		const FString& PlatformName,
		// const TArray<FString>& PakOptions,
		TMap<FString, FAssetDependenciesInfo>& ScanedCaches,
		const FExportPatchSettings* PatcheSettings=nullptr
	);

	static TArray<FString> GetPakCommandStrByCommands(const TArray<FPakCommand>& PakCommands, const TArray<FReplaceText>& InReplaceTexts = TArray<FReplaceText>{},bool bIoStore=false);

	static FHotPatcherAssetDependency GetAssetRelatedInfo(
		const FAssetDetail& InAsset,
		const TArray<EAssetRegistryDependencyTypeEx>& AssetRegistryDependencyTypes,
		TMap<FString, FAssetDependenciesInfo>& ScanedCaches
	);
	static TArray<FHotPatcherAssetDependency> GetAssetsRelatedInfo(
		const TArray<FAssetDetail>& InAssets,
		const TArray<EAssetRegistryDependencyTypeEx>& AssetRegistryDependencyTypes,
		TMap<FString, FAssetDependenciesInfo>& ScanedCaches
	);
	static TArray<FHotPatcherAssetDependency> GetAssetsRelatedInfoByFAssetDependencies(
		const FAssetDependenciesInfo& InAssetsDependencies,
		const TArray<EAssetRegistryDependencyTypeEx>& AssetRegistryDependencyTypes,
		TMap<FString, FAssetDependenciesInfo>& ScanedCaches
	);

	static bool GetCookProcCommandParams(const FCookerConfig& InConfig,FString& OutParams);
	//static bool SerializeMonolithicPathMode(const EMonolithicPathMode& InMode, TSharedPtr<FJsonValue>& OutJsonValue);
	//static bool DeSerializeMonolithicPathMode(const TSharedPtr<FJsonValue>& InJsonValue, EMonolithicPathMode& OutMode);

	static void ExcludeContentForVersionDiff(FPatchVersionDiff& VersionDiff,const TArray<FString>& ExcludeRules = {TEXT("")});

	template<typename ENUM_TYPE>
	static UEnum* GetUEnum()
	{
#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 21
		UEnum* FoundEnum = StaticEnum<ENUM_TYPE>();
#else
		FString EnumTypeName = ANSI_TO_TCHAR(UFlibPatchParserHelper::GetCPPTypeName<ENUM_TYPE>().c_str());
		UEnum* FoundEnum = FindObject<UEnum>(ANY_PACKAGE, *EnumTypeName, true); 
#endif
		return FoundEnum;
	}
	
	template<typename ENUM_TYPE>
	static FString GetEnumNameByValue(ENUM_TYPE InEnumValue, bool bFullName = false)
	{
		FString result;
		{
			FString TypeName;
			FString ValueName;

#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 21
			UEnum* FoundEnum = StaticEnum<ENUM_TYPE>();
#else
			FString EnumTypeName = ANSI_TO_TCHAR(UFlibPatchParserHelper::GetCPPTypeName<ENUM_TYPE>().c_str());
			UEnum* FoundEnum = FindObject<UEnum>(ANY_PACKAGE, *EnumTypeName, true); 
#endif
			if (FoundEnum)
			{
				result = FoundEnum->GetNameByValue((int64)InEnumValue).ToString();
				result.Split(TEXT("::"), &TypeName, &ValueName, ESearchCase::CaseSensitive, ESearchDir::FromEnd);
				if (!bFullName)
				{
					result = ValueName;
				}
			}
		}
		return result;
	}

	template<typename ENUM_TYPE>
	static bool GetEnumValueByName(const FString& InEnumValueName, ENUM_TYPE& OutEnumValue)
	{
		bool bStatus = false;

#if ENGINE_MAJOR_VERSION > 4 || ENGINE_MINOR_VERSION > 21
		UEnum* FoundEnum = StaticEnum<ENUM_TYPE>();
		FString EnumTypeName = FoundEnum->CppType;
#else
		FString EnumTypeName = ANSI_TO_TCHAR(UFlibPatchParserHelper::GetCPPTypeName<ENUM_TYPE>().c_str());
		UEnum* FoundEnum = FindObject<UEnum>(ANY_PACKAGE, *EnumTypeName, true); 
#endif
		if (FoundEnum)
		{
			FString EnumValueFullName = EnumTypeName + TEXT("::") + InEnumValueName;
			int32 EnumIndex = FoundEnum->GetIndexByName(FName(*EnumValueFullName));
			if (EnumIndex != INDEX_NONE)
			{
				int32 EnumValue = FoundEnum->GetValueByIndex(EnumIndex);
				ENUM_TYPE ResultEnumValue = (ENUM_TYPE)EnumValue;
				OutEnumValue = ResultEnumValue;
				bStatus = true;
			}
		}
		return bStatus;
	}

#if ENGINE_MAJOR_VERSION <= 4 && ENGINE_MINOR_VERSION <= 21
	template<typename T>
	static std::string GetCPPTypeName()
	{
		std::string result;
		std::string type_name = typeid(T).name();

		std::for_each(type_name.begin(),type_name.end(),[&result](const char& character){if(!std::isdigit(character)) result.push_back(character);});

		return result;
	}
#endif
	static FString MountPathToRelativePath(const FString& InMountPath);

	static FString SerializeAssetsDependencyAsJsonString(const TArray<FHotPatcherAssetDependency>& InAssetsDependency);
	static bool SerializePlatformPakInfoToString(const TMap<FString, TArray<FPakFileInfo>>& InPakFilesMap, FString& OutString);
	static bool SerializePlatformPakInfoToJsonObject(const TMap<FString, TArray<FPakFileInfo>>& InPakFilesMap, TSharedPtr<FJsonObject>& OutJsonObject);
	template<typename TStructType>
	static bool TSerializeStructAsJsonObject(const TStructType& InStruct,TSharedPtr<FJsonObject>& OutJsonObject)
	{
		if(!OutJsonObject.IsValid())
		{
			OutJsonObject = MakeShareable(new FJsonObject);
		}
		bool bStatus = FJsonObjectConverter::UStructToJsonObject(TStructType::StaticStruct(),&InStruct,OutJsonObject.ToSharedRef(),0,0);
		return bStatus;
	}

	template<typename TStructType>
    static bool TDeserializeJsonObjectAsStruct(const TSharedPtr<FJsonObject>& OutJsonObject,TStructType& InStruct)
	{
		bool bStatus = false;
		if(OutJsonObject.IsValid())
		{
			bStatus = FJsonObjectConverter::JsonObjectToUStruct(OutJsonObject.ToSharedRef(),TStructType::StaticStruct(),&InStruct,0,0);
		}
		return bStatus;
	}

	template<typename TStructType>
    static bool TSerializeStructAsJsonString(const TStructType& InStruct,FString& OutJsonString)
	{
		bool bRunStatus = false;

		{
			TSharedPtr<FJsonObject> JsonObject;
			if (UFlibPatchParserHelper::TSerializeStructAsJsonObject<TStructType>(InStruct,JsonObject) && JsonObject.IsValid())
			{
				auto JsonWriter = TJsonWriterFactory<>::Create(&OutJsonString);
				FJsonSerializer::Serialize(JsonObject.ToSharedRef(), JsonWriter);
				bRunStatus = true;
			}
		}
		return bRunStatus;
	}

	template<typename TStructType>
    static bool TDeserializeJsonStringAsStruct(const FString& InJsonString,TStructType& OutStruct)
	{
		bool bRunStatus = false;
		TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(InJsonString);
		TSharedPtr<FJsonObject> DeserializeJsonObject;
		if (FJsonSerializer::Deserialize(JsonReader, DeserializeJsonObject))
		{
			bRunStatus = UFlibPatchParserHelper::TDeserializeJsonObjectAsStruct<TStructType>(DeserializeJsonObject,OutStruct);
		}
		return bRunStatus;
	}

	static TMap<FString, FString> GetCommandLineParamsMap(const FString& CommandLine)
	{
		TMap<FString, FString> resault;
		TArray<FString> ParamsSwitchs, ParamsTokens;
		FCommandLine::Parse(*CommandLine, ParamsTokens, ParamsSwitchs);

		for (const auto& SwitchItem : ParamsSwitchs)
		{
			TArray<FString> SwitchArray;
			SwitchItem.ParseIntoArray(SwitchArray,TEXT("="),true);
			if(SwitchArray.Num()>1)
			{
				resault.Add(SwitchArray[0],SwitchArray[1].TrimQuotes());
			}
			else
			{
				resault.Add(SwitchArray[0],TEXT(""));
			}
		}
		return resault;
	}

	static bool HasPrroperty(UStruct* Field,const FString& FieldName)
	{
		for(TFieldIterator<FProperty> PropertyIter(Field);PropertyIter;++PropertyIter)
		{
			if(PropertyIter->GetName().Equals(FieldName,ESearchCase::IgnoreCase))
			{
				return true;
			}
		}
		return false;
	}

	
	template<typename T>
    static void ReplaceProperty(T& Struct, const TMap<FString, FString>& ParamsMap)
	{
		TSharedPtr<FJsonObject> DeserializeJsonObject;
		UFlibPatchParserHelper::TSerializeStructAsJsonObject(Struct,DeserializeJsonObject);
		if (DeserializeJsonObject.IsValid())
		{
			TArray<FString> MapKeys;
			ParamsMap.GetKeys(MapKeys);

			for(const auto& key:MapKeys)
			{
				TArray<FString> BreakedDot;
				key.ParseIntoArray(BreakedDot,TEXT("."));
				if(BreakedDot.Num())
				{
					TSharedPtr<FJsonObject> JsonObject = DeserializeJsonObject;
					if(HasPrroperty(T::StaticStruct(),BreakedDot[0]))
					{
						for(int32 index=0;index<BreakedDot.Num()-1;++index)
						{
							JsonObject = JsonObject->GetObjectField(BreakedDot[index]);
						}

						if(JsonObject)
						{
							JsonObject->SetStringField(BreakedDot[BreakedDot.Num()-1],*ParamsMap.Find(key));
						}
					}
				}
			}
			UFlibPatchParserHelper::TDeserializeJsonObjectAsStruct<T>(DeserializeJsonObject,Struct);
		}
	}
	
	template<typename T>
	static TArray<TArray<T>> SplitArray(const TArray<T>& Array,int32 SplitNum)
	{
		TArray<TArray<T>> result;
		result.AddDefaulted(SplitNum);

		for( int32 index=0; index<Array.Num(); ) 
		{
			for(auto& SplitItem:result)
			{
				SplitItem.Add(Array[index]);
				++index;
				if(index >= Array.Num())
				{
					break;
				}
			}
		}
		return result;
	}
	
	UFUNCTION(BlueprintCallable)
	static TArray<FAssetDetail> GetAllAssetDependencyDetails(
		const FAssetDetail& Asset,
		const TArray<EAssetRegistryDependencyTypeEx>& Types,
		const FString& AssetType,
		TMap<FString, FAssetDependenciesInfo>& ScanedCaches
	);

	static TMap<FString,FString> GetReplacePathMarkMap();
	static FString ReplaceMark(const FString& Src);
	static FString ReplaceMarkPath(const FString& Src);
	// [PORJECTDIR] to real path
	static void ReplacePatherSettingProjectDir(TArray<FPlatformExternAssets>& PlatformAssets);


	static TArray<FString> GetUnCookUassetExtensions();
	static TArray<FString> GetCookedUassetExtensions();
	static bool IsCookedUassetExtensions(const FString& InAsset);
	static bool IsUnCookUassetExtension(const FString& InAsset);
	// ../../../Content/xxxx.uasset to /Game/xxxx
	static FString UAssetMountPathToPackagePath(const FString& InAssetMountPath);

	static bool MatchStrInArray(const FString& InStr,const TArray<FString>& InArray);
	static FString LoadAESKeyStringFromCryptoFile(const FString& InCryptoJson);
	static FAES::FAESKey LoadAESKeyFromCryptoFile(const FString& InCryptoJson);
public:
	static bool GetPluginPakPathByName(const FString& PluginName,FString& uPluginAbsPath,FString& uPluginMountPath);
	// ../../../Example/Plugin/XXXX/
	static FString GetPluginMountPoint(const FString& PluginName);
	// [PRIJECTDIR]/AssetRegistry to ../../../Example/AssetRegistry
	static FString ParserMountPointRegular(const FString& Src);

public:
	UFUNCTION(BlueprintCallable)
	static void ReloadShaderbytecode();
	UFUNCTION(BlueprintCallable,Exec)
		static bool LoadShaderbytecode(const FString& LibraryName, const FString& LibraryDir);	
	UFUNCTION(BlueprintCallable,Exec)
		static void CloseShaderbytecode(const FString& LibraryName);

public:
	// Encrypt
	static FPakEncryptionKeys GetCryptoByProjectSettings();
	static FEncryptSetting GetCryptoSettingsByJson(const FString& CryptoJson);

	static FEncryptSetting GetCryptoSettingByPakEncryptSettings(const FPakEncryptSettings& Config);
	
	static bool SerializePakEncryptionKeyToFile(const FPakEncryptionKeys& PakEncryptionKeys,const FString& ToFile);

	static TArray<FDirectoryPath> GetDefaultForceSkipContentDir();
};

