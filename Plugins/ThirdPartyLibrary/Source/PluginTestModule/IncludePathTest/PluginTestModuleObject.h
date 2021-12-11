// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "PluginTestModule/PluginTestModule.h"
#include "PluginTestModuleObject.generated.h"
/**
 * 
 */
UCLASS()
class  UPluginTestModuleObject : public UObject
{
	GENERATED_BODY()
public:
	UPluginTestModuleObject();

	void Fire();
};
