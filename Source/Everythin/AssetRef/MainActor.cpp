// Fill out your copyright notice in the Description page of Project Settings.


#include "MainActor.h"
#include "Engine/AssetManager.h"
#include "../Everythin.h"
#include "UObject/GCObjectScopeGuard.h"
// Sets default values
AMainActor::AMainActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	static ConstructorHelpers::FClassFinder<AActorOne> ObjOne(TEXT("Blueprint'/Game/AssetRef/ActorOne_BP.ActorOne_BP_C'"));
	if (ObjOne.Succeeded())
	{
		actorOneClass = ObjOne.Class;
	}

	static ConstructorHelpers::FClassFinder<AActorTwo> ObjTwo(TEXT("Blueprint'/Game/AssetRef/ActorTwo_BP.ActorTwo_BP_C'")); 
	if(ObjTwo.Succeeded())
		actorTwo = ObjTwo.Class;
}

// Called when the game starts or when spawned2
void AMainActor::BeginPlay()
{
	Super::BeginPlay();
	actorOne = GWorld->SpawnActor<AActorOne>(actorOneClass, FVector::ZeroVector, FRotator::ZeroRotator);

	{
		auto x = actorOne->GetOwner();
		//actorOne = nullptr;//这样也不会被销毁，ulevel里有个actors数组 spawn的actor存在那里面，所以不会被垃圾回收。销毁需要调用Destroy
		actorOne->Destroy();//从ulevel的actors中删除，还会调用MarkPendingKill
		auto isVd = actorOne->IsValidLowLevel();//True why?
		actorOne = nullptr;//消除野指针
		GEngine->ForceGarbageCollection(true);
	}


	{
		const AActorTwo* GladOS = NewObject<AActorTwo>();
		FGCObjectScopeGuard scop(GladOS);//这里虽然能保住GladOS不被销毁，但是出了这个{}的作用域，随着scop的销毁，GladOS照样会被销毁。目前没弄清楚FGCObjectScopeGuard的用法
		auto isVd = GladOS->IsValidLowLevel();
		GEngine->ForceGarbageCollection(true);
	}

	UMaterial* obj = MatSoftPtr.Get();
	UObject* obj2 =  MatSoft.ResolveObject();
	UAssetManager::GetStreamableManager().LoadSynchronous(MatSoft);
	obj = MatSoftPtr.Get();
	obj2 = MatSoft.ResolveObject();

	if (obj == obj2) {
		UE_LOG(Everythin, Warning, TEXT("Has Load and same"));
	}
}

// Called every frame
void AMainActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

