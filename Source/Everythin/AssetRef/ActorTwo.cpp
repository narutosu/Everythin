// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorTwo.h"

// Sets default values
AActorTwo::AActorTwo()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

AActorTwo::~AActorTwo()
{
	int x = 12;
	x = 23;
}

// Called when the game starts or when spawned
void AActorTwo::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AActorTwo::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

