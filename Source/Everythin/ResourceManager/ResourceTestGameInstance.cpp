// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceTestGameInstance.h"
#include "ResourceManager.h"

UResourceTestGameInstance::UResourceTestGameInstance()
{
	m_ResourceManager = NewObject<UResourceManager>();
}

UResourceManager* UResourceTestGameInstance::GetResourceManager()
{
	return m_ResourceManager;
}
