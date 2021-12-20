// Fill out your copyright notice in the Description page of Project Settings.


#include "PatchTestGameInstance.h"

#include "ChunkDownloader.h"
#include "Misc/CoreDelegates.h"
#include "AssetRegistryModule.h"

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