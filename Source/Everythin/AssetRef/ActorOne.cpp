// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorOne.h"

// Sets default values
AActorOne::AActorOne()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void AActorOne::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AActorOne::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

