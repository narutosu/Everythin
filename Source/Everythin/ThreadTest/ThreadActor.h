// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/EngineTypes.h"
#include "ThreadActor.generated.h"
UCLASS()
class EVERYTHIN_API AThreadActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AThreadActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	class RunObject* myRunObject;
	FTimerHandle timeHandle;
	class FRunnableThread* myThread;

	UFUNCTION()
		void OnTimer();
};