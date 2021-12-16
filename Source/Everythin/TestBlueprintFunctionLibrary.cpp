// Fill out your copyright notice in the Description page of Project Settings.


#include "TestBlueprintFunctionLibrary.h"
#include "UObject/UObjectGlobals.h"
#include "Everythin.h"

void UTestBlueprintFunctionLibrary::LoadPakComplete()
{

}

void UTestBlueprintFunctionLibrary::PrintObject(FText objPath)
{
	TArray<UObject*> Results;
	UObject* get = LoadObject<UObject>(nullptr, *objPath.ToString());
	GetObjectsWithOuter(get, Results);
	UE_LOG(Everythin, Warning, TEXT("print Pachage: %s"), *get->GetName());
	for (auto item : Results) {
		UE_LOG(Everythin, Warning, TEXT("objectName: %s"), *item->GetName());
	}

}
