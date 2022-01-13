// Fill out your copyright notice in the Description page of Project Settings.


#include "TestBlueprintFunctionLibrary.h"
#include "UObject/UObjectGlobals.h"
#include "Everythin.h"
#include "ProfilingDebugging/ScopedTimers.h"
#include "IPlatformFilePak.h"
#include "PatchTest/PatchTestGameInstance.h"
#include <string>
#include "Stats/Stats.h"

UPatchTestGameInstance* UTestBlueprintFunctionLibrary::Instance = nullptr;

/*定义分组
@TEXT("Sugen_Desc") 分组描述，不关键
@STATGROUP_SugenStat 分组id,在统计工具上会看到分组“SugenStat”
@STATCAT_Advanced 分组的分组，即在游戏视口左上角箭头→stats的几个大选项
*/
DECLARE_STATS_GROUP(TEXT("Sugen_Desc"), STATGROUP_SugenStat, STATCAT_Advanced);


/*定义埋点，这里定义了三个。埋点是属于上面的分组的。
@TEXT("SugenBegin") 埋点描述，在统计工具上会看到
@STAT_SugenBegin 埋点id,代码里打点的时候会用到
@STATGROUP_SugenStat 上方定义的分组id。表示属于上面这个分组。
*/
DECLARE_CYCLE_STAT(TEXT("SugenBegin"), STAT_SugenBegin, STATGROUP_SugenStat);
DECLARE_CYCLE_STAT(TEXT("Sugen1"), STAT_Sugen1, STATGROUP_SugenStat);
DECLARE_CYCLE_STAT(TEXT("Sugen2"), STAT_Sugen2, STATGROUP_SugenStat);

void UTestBlueprintFunctionLibrary::LoadPakComplete()
{

}

void UTestBlueprintFunctionLibrary::PrintObject(FText objPath)
{
	/*打下埋点，定义了一个对象。此对象在函数结束时销毁，从而统计出函数执行时间
	@STAT_SugenBegin 上面定义的埋点id。
	*/
	SCOPE_CYCLE_COUNTER(STAT_SugenBegin);

	double PrintObjecttime = 0.0;
	{
		FScopedDurationTimer Timer(PrintObjecttime);
		auto start = clock();
		TArray<UObject*> Results;
		SCOPE_CYCLE_COUNTER(STAT_Sugen1);
		UObject* get = LoadObject<UObject>(nullptr, *objPath.ToString());
		SCOPE_CYCLE_COUNTER(STAT_Sugen2);
		if (get) {
			UE_LOG(Everythin, Warning, TEXT("loadCast: %d"), clock() - start);
			GetObjectsWithOuter(get, Results);
			UE_LOG(Everythin, Warning, TEXT("print Pachage: %s"), *get->GetName());
		}
	}
	UE_LOG(Everythin, Warning, TEXT("PrintObjecttime == %lf"), PrintObjecttime);
	
	
} 

void UTestBlueprintFunctionLibrary::MountTest()
{
	double MountTestTime = 0.0;
	{
		//FPakFile PakFile(PakPlatformFile,*SavePakDir, false);
		FString MountPoint(FPaths::ProjectContentDir()); //"/../../../Engine/Content/"对应路径  

		for (int i = 0; i < 3000; i++) {
			FString SavePakDir4 = FPaths::ProjectContentDir() + TEXT("PaksTest/pakchunk1007-WindowsNoEditor_") + UTF8_TO_TCHAR(std::to_string(i).c_str()) + TEXT("_p.pak");
			MountPak(*SavePakDir4, 0);
		}
	}
	UE_LOG(Everythin, Warning, TEXT("MountTestTime == %lf"), MountTestTime);
}

bool UTestBlueprintFunctionLibrary::MountPak(const FString& PakPath, int32 PakOrder, const FString& InMountPoint)
{
	bool bMounted = false;
#if !WITH_EDITOR
	FPakPlatformFile* PakFileMgr = (FPakPlatformFile*)FPlatformFileManager::Get().GetPlatformFile(FPakPlatformFile::GetTypeName());
	if (!PakFileMgr)
	{
		UE_LOG(LogTemp, Log, TEXT("GetPlatformFile(TEXT(\"PakFile\") is NULL"));
		return false;
	}

	PakOrder = FMath::Max(0, PakOrder);

	if (FPaths::FileExists(PakPath) && FPaths::GetExtension(PakPath) == TEXT("pak"))
	{
		const TCHAR* MountPount = NULL;
		if(!InMountPoint.IsEmpty())
			MountPount = InMountPoint.GetCharArray().GetData();
		if (PakFileMgr->Mount(*PakPath, PakOrder, MountPount))
		{
			UE_LOG(LogTemp, Log, TEXT("Mounted = %s, Order = %d, MountPoint = %s"), *PakPath, PakOrder, !MountPount ? TEXT("(NULL)") : MountPount);
			bMounted = true;
		}
		else {
			UE_LOG(LogTemp, Error, TEXT("Faild to mount pak = %s"), *PakPath);
			bMounted = false;
		}
	}

#endif
	return bMounted;
}