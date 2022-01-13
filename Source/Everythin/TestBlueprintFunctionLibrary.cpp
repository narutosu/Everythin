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

/*�������
@TEXT("Sugen_Desc") �������������ؼ�
@STATGROUP_SugenStat ����id,��ͳ�ƹ����ϻῴ�����顰SugenStat��
@STATCAT_Advanced ����ķ��飬������Ϸ�ӿ����ϽǼ�ͷ��stats�ļ�����ѡ��
*/
DECLARE_STATS_GROUP(TEXT("Sugen_Desc"), STATGROUP_SugenStat, STATCAT_Advanced);


/*������㣬���ﶨ�����������������������ķ���ġ�
@TEXT("SugenBegin") �����������ͳ�ƹ����ϻῴ��
@STAT_SugenBegin ���id,���������ʱ����õ�
@STATGROUP_SugenStat �Ϸ�����ķ���id����ʾ��������������顣
*/
DECLARE_CYCLE_STAT(TEXT("SugenBegin"), STAT_SugenBegin, STATGROUP_SugenStat);
DECLARE_CYCLE_STAT(TEXT("Sugen1"), STAT_Sugen1, STATGROUP_SugenStat);
DECLARE_CYCLE_STAT(TEXT("Sugen2"), STAT_Sugen2, STATGROUP_SugenStat);

void UTestBlueprintFunctionLibrary::LoadPakComplete()
{

}

void UTestBlueprintFunctionLibrary::PrintObject(FText objPath)
{
	/*������㣬������һ�����󡣴˶����ں�������ʱ���٣��Ӷ�ͳ�Ƴ�����ִ��ʱ��
	@STAT_SugenBegin ���涨������id��
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
		FString MountPoint(FPaths::ProjectContentDir()); //"/../../../Engine/Content/"��Ӧ·��  

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