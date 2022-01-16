// Fill out your copyright notice in the Description page of Project Settings.


#include "TaskActor.h"
#include "TestTask.h"
// Sets default values
ATaskActor::ATaskActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned2
void ATaskActor::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ATaskActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ATaskActor::RunPrimeTask(int32 num_primes)
{
	(new FAutoDeleteAsyncTask<FTestTask>(num_primes))->StartBackgroundTask();
}

void ATaskActor::RunPrimeTaskOnMain(int32 num_primes)
{
// 	FTestTask* task = new FTestTask(num_primes);
// 	task->DoWorkMain();
// 	delete task;
}
