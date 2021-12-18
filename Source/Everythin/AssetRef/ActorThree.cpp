// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorThree.h"

// Sets default values
AActorThree::AActorThree()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AActorThree::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AActorThree::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

