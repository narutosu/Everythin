// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TestModuleObject.generated.h"
DECLARE_LOG_CATEGORY_EXTERN(TestModuleLog, All, All);
/**
 * 
 */
UCLASS()
class TESTMODULE_API UTestModuleObject : public UObject
{
	GENERATED_BODY()
public:
	UTestModuleObject();

	void Fire();
};
