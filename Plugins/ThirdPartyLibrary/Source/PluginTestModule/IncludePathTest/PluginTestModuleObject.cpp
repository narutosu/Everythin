// Fill out your copyright notice in the Description page of Project Settings.

#include "PluginTestModuleObject.h"

UPluginTestModuleObject::UPluginTestModuleObject()
{
	
}

void UPluginTestModuleObject::Fire()
{
	UE_LOG(PluginTestModuleLog, Warning, TEXT("UPluginTestModuleObject fire"));
}
