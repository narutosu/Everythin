// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "TestBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class EVERYTHIN_API UTestBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Pak")
		static void LoadPakComplete();

	UFUNCTION(BlueprintCallable, Category = "Print")
		static void PrintObject(FText objPath);

	UFUNCTION(BlueprintCallable, Category = "Print")
		static void MountTest();

	static bool MountPak(const FString& PakPath, int32 PakOrder, const FString& InMountPoint = FString());

	static class UPatchTestGameInstance* Instance;
};
