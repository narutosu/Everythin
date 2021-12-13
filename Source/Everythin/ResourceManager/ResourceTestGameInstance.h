// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "ResourceTestGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class EVERYTHIN_API UResourceTestGameInstance : public UGameInstance
{
	GENERATED_BODY()
	UResourceTestGameInstance();
public:
	UFUNCTION(BlueprintCallable,Category="Study")
	UResourceManager*  GetResourceManager();
private:
	UPROPERTY()
	class UResourceManager* m_ResourceManager;
};
