// Copyright Epic Games, Inc. All Rights Reserved.

#include "TestModule.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(TestModuleLog);

#define LOCTEXT_NAMESPACE "FTestModule"

void FTestModule::StartupModule()
{
	UE_LOG(TestModuleLog, Warning, TEXT("FTestModule module has started!"));

}

void FTestModule::ShutdownModule()
{
	UE_LOG(TestModuleLog, Warning, TEXT("FTestModule module has shut down"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_GAME_MODULE( FTestModule, TestModule);
