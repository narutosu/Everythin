// Fill out your copyright notice in the Description page of Project Settings.


#include "PatchTestGameInstance.h"

#include "ChunkDownloader.h"
#include "Misc/CoreDelegates.h"
#include "AssetRegistryModule.h"
#include "Engine/StreamableManager.h"
#include "../AssetRef/ActorTwo.h"
#include "IPlatformFilePak.h"

void UPatchTestGameInstance::Init()
{
    Super::Init();

    //DefaultGame.ini中配置的url。本例是 cdn-framw.h3d.com.cn/framw_hot_update/UE4/EverythinCDN。
    const FString DeploymentName = "H3D_DeploymentName_EverythinLive";
    //在上面的url目录下寻找文件夹，文件夹名为 EverythinKey。
    //更新是以这个id作为更新路径，并用它进行更新检查（是否已经更新），本地id相同则跳过更新
    const FString ContentBuildId = "EverythinKey";

    //获取FChunkDownloader单例
    TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetOrCreate();
    /* 
    参数：初始化更新平台："Windows" 最大同时下载数量：8
    执行过程：在FPaths::ProjectPersistentDownloadDir()/PakCache目录下; 检查LocalManifest.txt跟相同目录的.pak文件是否一致
    一致的标准是 pak名字跟大小一致。不一致的删除pak，并且在LocalManifest.txt中删除对应行。
    经过这那一步，LocalManifest.txt中跟本地pak已经一致
    */
    Downloader->Initialize("Windows", 8);

    /*对比 CachedBuildManifest.txt跟LocalManifest.txt中的列表
    在LocalManifest.txt中删除CachedBuildManifest.txt中没有的pak,并更新LocalManifest.txt
    对CachedBuildManifest.txt中的pak创建chunk任务。（Chunks跟PakFiles变量）。供后续步骤执行。
    */
    Downloader->LoadCachedBuild(DeploymentName);

    // 下载远程 BuildManifest-{platformName}.txt文件。格式固定且写死。本例为 BuildManifest-Windows.txt
    TFunction<void(bool bSuccess)> UpdateCompleteCallback = [&](bool bSuccess) {
        UE_LOG(LogTemp, Display, TEXT("OnManifestUpdateComplete lambda"));
        OnManifestUpdateComplete(bSuccess); 
    };
    Downloader->UpdateBuild(DeploymentName, ContentBuildId, UpdateCompleteCallback);
    auto x = 12;

    MountPakTest();
}


void UPatchTestGameInstance::Shutdown()
{
    Super::Shutdown();

    // Shut down ChunkDownloader
    FChunkDownloader::Shutdown();
}

bool UPatchTestGameInstance::PatchGame()
{
    // make sure the download manifest is up to date
    if (bIsDownloadManifestUpToDate)
    {
        // get the chunk downloader
        TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

        // report current chunk status
        for (int32 ChunkID : ChunkDownloadList)
        {
            int32 ChunkStatus = static_cast<int32>(Downloader->GetChunkStatus(ChunkID));
            UE_LOG(LogTemp, Display, TEXT("Chunk %i status: %i"), ChunkID, ChunkStatus);
        }

        /*
        根据下载的Manifest.txt分析出还需要下载的文件并下载
        */
        TFunction<void (bool bSuccess)> DownloadCompleteCallback = [&](bool bSuccess){
            UE_LOG(LogTemp, Warning, TEXT("OnDownloadComplete %d"),bSuccess);
            OnDownloadComplete(bSuccess);
        };
        Downloader->DownloadChunks(ChunkDownloadList, DownloadCompleteCallback, 1);

        /*
        此调用暂时没明白其用意。看着像是跟下载进度相关。但是无此调用也能获取进度。注释掉也不影响功能
        */
		TFunction<void(bool bSuccess)> LoadingModeCompleteCallback = [&](bool bSuccess) {
			UE_LOG(LogTemp, Warning, TEXT("OnLoadingModeComplete %d"), bSuccess);
			OnLoadingModeComplete(bSuccess);
		};
		Downloader->BeginLoadingMode(LoadingModeCompleteCallback);
        return true;
    }

    // we couldn't contact the server to validate our manifest, so we can't patch
    UE_LOG(LogTemp, Display, TEXT("Manifest Update Failed. Can't patch the game"));
    return false;
}

void UPatchTestGameInstance::OnManifestUpdateComplete(bool bSuccess)
{
    UE_LOG(LogTemp, Display, TEXT("OnManifestUpdateComplete %d"), bSuccess);
    bIsDownloadManifestUpToDate = bSuccess;
    if(bIsDownloadManifestUpToDate)
        PatchGame();
}


void UPatchTestGameInstance::OnDownloadComplete(bool bSuccess)
{
#if WITH_EDITOR
    OnMountCompleteDele.Broadcast(true);
#else
if (bSuccess)
    {
        UE_LOG(LogTemp, Display, TEXT("Download complete"));

        // get the chunk downloader
        TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

        FJsonSerializableArrayInt DownloadedChunks;

        for (int32 ChunkID : ChunkDownloadList)
        {
            DownloadedChunks.Add(ChunkID);
        }

        //挂载下载的pak包。
        TFunction<void(bool bSuccess)> MountCompleteCallback = [&](bool bSuccess){OnMountComplete(bSuccess);};
        Downloader->MountChunks(DownloadedChunks, MountCompleteCallback);

		OnPatchComplete.Broadcast(true);
    }
    else
    {

        UE_LOG(LogTemp, Display, TEXT("Load process failed"));

        // call the delegate
        OnPatchComplete.Broadcast(false);
    }
#endif
}

void UPatchTestGameInstance::GetLoadingProgress(int32& BytesDownloaded, int32& TotalBytesToDownload, float& DownloadPercent, int32& ChunksMounted, int32& TotalChunksToMount, float& MountPercent) const
{
    //Get a reference to ChunkDownloader
    TSharedRef<FChunkDownloader> Downloader = FChunkDownloader::GetChecked();

    //Get the loading stats struct
    FChunkDownloader::FStats LoadingStats = Downloader->GetLoadingStats();

    //Get the bytes downloaded and bytes to download
    BytesDownloaded = LoadingStats.BytesDownloaded;
    TotalBytesToDownload = LoadingStats.TotalBytesToDownload;

    //Get the number of chunks mounted and chunks to download
    ChunksMounted = LoadingStats.ChunksMounted;
    TotalChunksToMount = LoadingStats.TotalChunksToMount;

    //Calculate the download and mount percent using the above stats
    DownloadPercent = (float)BytesDownloaded / (float)TotalBytesToDownload;
    MountPercent = (float)ChunksMounted / (float)TotalChunksToMount;
}

void UPatchTestGameInstance::OnLoadingModeComplete(bool bSuccess)
{
    OnDownloadComplete(bSuccess);
}

void UPatchTestGameInstance::OnMountComplete(bool bSuccess)
{
    OnMountCompleteDele.Broadcast(bSuccess);
}

void UPatchTestGameInstance::MountPakTest()
{
 //#if IS_PROGRAM
    FString SavePakDir0 = FPaths::ProjectSavedDir() + TEXT("testPak_p.pak");
    FString SavePakDir1 = FPaths::ProjectSavedDir() + TEXT("testPak_001_p.pak");
    FString SavePakDir = FPaths::ProjectSavedDir() + TEXT("testPak_002_p.pak");
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

    FPakPlatformFile* PakPlatformFile = new FPakPlatformFile();
    PakPlatformFile->Initialize(&PlatformFile, TEXT(""));
    FPlatformFileManager::Get().SetPlatformFile(*PakPlatformFile);


    //FPakFile PakFile(PakPlatformFile,*SavePakDir, false);

	FString MountPoint(FPaths::ProjectContentDir()); //"/../../../Engine/Content/"对应路径  
	//PakFile.SetMountPoint(*MountPoint);
	//对pak文件mount到前面设定的MountPoint  
	if (PakPlatformFile->Mount(*SavePakDir0, 0, *MountPoint))
	{
		UE_LOG(LogClass, Log, TEXT("Mount Success testPak_p"));
	}
	else
	{
		UE_LOG(LogClass, Error, TEXT("Mount Failed testPak_p"));
	}
    if (PakPlatformFile->Mount(*SavePakDir1, 0, *MountPoint))
    {
        UE_LOG(LogClass, Log, TEXT("Mount Success testPak_001_p"));
    }
	else
	{
		UE_LOG(LogClass, Error, TEXT("Mount Failed testPak_001_p"));
	}
	if (PakPlatformFile->Mount(*SavePakDir, 0, *MountPoint))
	{
		UE_LOG(LogClass, Log, TEXT("Mount Success testPak_002_p"));
		TArray<FString> FileList;
		//得到Pak文件中MountPoint路径下的所有文件  
// 		PakFile.FindFilesAtPath(FileList, *PakFile.GetMountPoint(), true, false, true);
// 		FStreamableManager StreamableManager;
// 		//对文件的路径进行处理,转换成StaticLoadObject的那种路径格式  
// 		FString AssetName = FileList[0];
// 
// 		UE_LOG(LogClass, Log, TEXT("Mount Success AssetName：%s "), *AssetName);
// 		FString AssetShortName = FPackageName::GetShortName(AssetName);
// 		FString LeftStr;
// 		FString RightStr;
// 		AssetShortName.Split(TEXT("."), &LeftStr, &RightStr);
// 		AssetName = TEXT("/Engine/") + LeftStr + TEXT(".") + LeftStr;    //我们加载的时候用的是这个路径  
//         FSoftObjectPath reference = AssetName;
// 		//加载UObject  
// 		TSharedPtr<FStreamableHandle> LoadObjectHandle = StreamableManager.RequestSyncLoad(reference);
// 		if (LoadObjectHandle->GetLoadedAsset() != nullptr)
// 		{
// 			UE_LOG(LogClass, Log, TEXT("Object Load Success..."))
// 				//TheLoadObject = LoadObject;
// 		}
// 		else
// 		{
// 			UE_LOG(LogClass, Log, TEXT("Can not Load asset..."))
// 		}
	}
	else
	{
		UE_LOG(LogClass, Error, TEXT("Mount Failed testPak_002_p"));
	}
//#endif
}