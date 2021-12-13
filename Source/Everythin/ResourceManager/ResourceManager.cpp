// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceManager.h"
#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#include "Editor/UnrealEdEngine.h"
#else
#include "Engine/GameEngine.h"
#endif

DEFINE_LOG_CATEGORY(ResourceManagerLog);

UResourceManager::UResourceManager()
{
}


UResourceManager::~UResourceManager()
{
}


bool UResourceManager::SeamlessTravel(FString name)
{
	if (m_GameInstance == nullptr)
	{
		m_GameInstance = GetWorld()->GetGameInstance();
	}

	UE_LOG(ResourceManagerLog, Warning, TEXT("ResourceManager::ServerTravel, path, %s"), *name);
	this->m_GameInstance->GetWorld()->SeamlessTravel(name);
	return true;
}

