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
		//actorOne = nullptr;//����Ҳ���ᱻ���٣�ulevel���и�actors���� spawn��actor���������棬���Բ��ᱻ�������ա�������Ҫ����Destroy
		actorOne->Destroy();//��ulevel��actors��ɾ�����������MarkPendingKill
		auto isVd = actorOne->IsValidLowLevel();//True why?
		actorOne = nullptr;//����Ұָ��
		GEngine->ForceGarbageCollection(true);
	}


	{
		const AActorTwo* GladOS = NewObject<AActorTwo>();
		FGCObjectScopeGuard scop(GladOS);//������Ȼ�ܱ�סGladOS�������٣����ǳ������{}������������scop�����٣�GladOS�����ᱻ���١�ĿǰûŪ���FGCObjectScopeGuard���÷�
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

