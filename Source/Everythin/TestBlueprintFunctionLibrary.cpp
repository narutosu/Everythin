// Fill out your copyright notice in the Description page of Project Settings.


#include "TestBlueprintFunctionLibrary.h"
#include "UObject/UObjectGlobals.h"
#include "Everythin.h"
#include "ProfilingDebugging/ScopedTimers.h"
#include "IPlatformFilePak.h"
#include <string>

void UTestBlueprintFunctionLibrary::LoadPakComplete()
{

}

void UTestBlueprintFunctionLibrary::PrintObject(FText objPath)
{
	double PrintObjecttime = 0.0;
	{
		FScopedDurationTimer Timer(PrintObjecttime);
		auto start = clock();
		TArray<UObject*> Results;
		UObject* get = LoadObject<UObject>(nullptr, *objPath.ToString());
		if (get) {
			UE_LOG(Everythin, Warning, TEXT("loadCast: %d"), clock() - start);
			GetObjectsWithOuter(get, Results);
			UE_LOG(Everythin, Warning, TEXT("print Pachage: %s"), *get->GetName());
		}
// 		for (auto item : Results) {
// 			UE_LOG(Everythin, Warning, TEXT("objectName: %s"), *item->GetName());
// 		}
	}
	UE_LOG(Everythin, Warning, TEXT("PrintObjecttime == %lf"), PrintObjecttime);
	
	
} 

void UTestBlueprintFunctionLibrary::MountTest()
{
	double MountTestTime = 0.0;
	{
		FScopedDurationTimer Timer(MountTestTime);

		IPlatformFile* PlatformFile = &FPlatformFileManager::Get().GetPlatformFile();
		TSharedPtr<FPakPlatformFile>  PakPlatformFile = MakeShareable(new FPakPlatformFile());
		PakPlatformFile->Initialize(PlatformFile, TEXT(""));
		FPlatformFileManager::Get().SetPlatformFile(*PakPlatformFile);

		//FPakFile PakFile(PakPlatformFile,*SavePakDir, false);

		FString MountPoint(FPaths::ProjectContentDir()); //"/../../../Engine/Content/"¶ÔÓ¦Â·¾¶  

		for (int i = 0; i < 3000; i++) {
			FString SavePakDir4 = FPaths::ProjectContentDir() + TEXT("PaksTest/pakchunk1007-WindowsNoEditor_") + UTF8_TO_TCHAR(std::to_string(i).c_str()) + TEXT("_p.pak");
			PakPlatformFile->Mount(*SavePakDir4, 0);
		}
	}
	UE_LOG(Everythin, Warning, TEXT("MountTestTime == %lf"), MountTestTime);
}
