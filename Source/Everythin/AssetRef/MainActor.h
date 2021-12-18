// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ActorOne.h"
#include "Templates/SubclassOf.h"
#include "ActorThree.h"
#include "ActorFour.h"
#include "ActorTwo.h"
#include "MainActor.generated.h"
UCLASS()
class EVERYTHIN_API AMainActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMainActor();
public:
	TSubclassOf<AActorOne> actorOneClass;
    UPROPERTY(EditDefaultsOnly, Category=Study)
		AActorOne* actorOne;
	UPROPERTY(EditDefaultsOnly, Category = Study)
		TSubclassOf<AActorTwo> actorTwo;
	UPROPERTY(EditDefaultsOnly, Category = Study)
		FSoftObjectPath actorThree;
	UPROPERTY(EditDefaultsOnly, Category = Study)
		FSoftClassPath actorFour;

	UPROPERTY(EditDefaultsOnly, Category = Study)
		FSoftObjectPath MatSoft;
	UPROPERTY(EditDefaultsOnly, Category = Study)
		TSoftObjectPtr<UMaterial> MatSoftPtr;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
