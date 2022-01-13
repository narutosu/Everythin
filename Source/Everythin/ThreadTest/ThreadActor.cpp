// Fill out your copyright notice in the Description page of Project Settings.


#include "ThreadActor.h"
#include "RunObject.h"
// Sets default values
AThreadActor::AThreadActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned2
void AThreadActor::BeginPlay()
{
	Super::BeginPlay();

	myRunObject = new RunObject();
	myThread = FRunnableThread::Create(myRunObject,TEXT("myRunObject"));
	GetWorldTimerManager().SetTimer(timeHandle,this,&AThreadActor::OnTimer,5.0f);
}

// Called every frame
void AThreadActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AThreadActor::OnTimer()
{
	if (myThread) {
		myThread->Kill(true);
		delete myRunObject;
		delete myThread;
	}
}
