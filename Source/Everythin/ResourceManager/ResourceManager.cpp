// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceManager.h"
#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#include "Editor/UnrealEdEngine.h"
#else
#include "Engine/GameEngine.h"
#endif
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(ResourceManagerLog);

UResourceManager::UResourceManager()
{
	if (m_GameInstance == nullptr)
	{
		m_GameInstance = GWorld ? GWorld->GetGameInstance() : nullptr;
		//m_GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
		//m_GameInstance = GetWorld()->GetGameInstance();
	}
}


UResourceManager::~UResourceManager()
{
}


bool UResourceManager::SeamlessTravel(FString name)
{
	UE_LOG(ResourceManagerLog, Warning, TEXT("ResourceManager::SeamlessTravel, path, %s"), *name);
	GWorld->SeamlessTravel(name);
	return true;
}

bool UResourceManager::SyncTravel(FString name)
{
	UE_LOG(ResourceManagerLog, Warning, TEXT("ResourceManager::SyncTravel, path, %s"), *name);
	UGameplayStatics::OpenLevel(GWorld, FName(*name));
	return true;
}


