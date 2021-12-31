// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceManager.h"
#if WITH_EDITOR
#include "Editor/EditorEngine.h"
#include "Editor/UnrealEdEngine.h"
#else
#include "Engine/GameEngine.h"
#endif
#include "Kismet/GameplayStatics.h"
#include "MoviePlayer/Public/MoviePlayer.h"
#include "Blueprint/UserWidget.h"
#include "AssetRegistryModule.h"
#include "ActionRPGLoadingScreen.h"

DEFINE_LOG_CATEGORY(ResourceManagerLog);

UResourceManager::UResourceManager()
{
#if WITH_EDITOR
	FCoreUObjectDelegates::OnAssetLoaded.AddUObject(this, &UResourceManager::OnAssetLoaded);
#endif

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	// 当资源新增时会调用FMyClass::OnAsssetAdded
	AssetRegistryModule.Get().OnAssetAdded().AddStatic(&UResourceManager::OnAssetAdded);
	TArray<FAssetData> data;
	AssetRegistryModule.Get().GetAllAssets(data);
	UE_LOG(ResourceManagerLog, Warning, TEXT("AssetRegistryModule data.Num() %d"), data.Num());

	if (m_GameInstance == nullptr)
	{
		m_GameInstance = GWorld ? GWorld->GetGameInstance() : nullptr;
		//m_GameInstance = UGameplayStatics::GetGameInstance(GetWorld());
		//m_GameInstance = GetWorld()->GetGameInstance();
	}

	//FCoreUObjectDelegates::PreLoadMap.AddUObject(this,&UResourceManager::BeginLoadingScreen);
	//FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(this, &UResourceManager::EndLoadingScreen);
	// 加载AssetRegistry


}


UResourceManager::~UResourceManager()
{
	//FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	//AssetRegistryModule.Get().OnAssetAdded().RemoveAll(this);
}


bool UResourceManager::SeamlessTravel(FString name)
{
	UE_LOG(ResourceManagerLog, Warning, TEXT("ResourceManager::SeamlessTravel, path, %s"), *name);
	GWorld->SeamlessTravel(name);
	return true;
}

bool UResourceManager::SyncTravel(FString name)
{
	UE_LOG(ResourceManagerLog, Warning, TEXT("ResourceManager::SyncTravel, path, %s"), *name);
	UGameplayStatics::OpenLevel(GWorld, FName(*name));
	return true;
}

void UResourceManager::BeginLoadingScreen(const FString& MapName)
{
	UE_LOG(ResourceManagerLog, Warning, TEXT("BeginLoadingScreen %s"), *MapName);
	FLoadingScreenAttributes LoadingScreen;
	LoadingScreen.bAutoCompleteWhenLoadingCompletes = false;
	LoadingScreen.bWaitForManualStop = true;
	LoadingScreen.bAllowEngineTick = true;

	LoadingScreen.bMoviesAreSkippable = true;//任意键打断如果加载完成
	//LoadingScreen.bWaitForManualStop = true;//
	LoadingScreen.MinimumLoadingScreenDisplayTime = 10.f;
	//LoadingScreen.PlaybackType = EMoviePlaybackType::MT_Looped;
// 	TSubclassOf<UUserWidget> umgClass1 = LoadClass<UUserWidget>(this, TEXT("/Game/seamlessTravel/LoadingScreamUmg.LoadingScreamUmg_C"));
// 	TSubclassOf<UUserWidget> umgClass2 = m_StreamableManager.LoadSynchronous<UClass>(FSoftObjectPath("/Game/seamlessTravel/LoadingScreamUmg.LoadingScreamUmg_C"));
// 	//不知道怎么destroy..
// 	m_GameInstance = GWorld ? GWorld->GetGameInstance() : nullptr;
// 	UUserWidget* inWidget = CreateWidget<UUserWidget>(m_GameInstance, umgClass1);//应该有垃圾回收
// 	TSharedPtr<SWidget> WidgetPtr = inWidget->TakeWidget();
	LoadingScreen.WidgetLoadingScreen = SNew(SRPGLoadingScreen);

	LoadingScreen.MoviePaths.Add("squad_intro_movie");
	GetMoviePlayer()->SetupLoadingScreen(LoadingScreen);
}

void UResourceManager::EndLoadingScreen(UWorld* world)
{
	UE_LOG(ResourceManagerLog, Warning, TEXT("EndLoadingScreen %s"), *world->GetName());
	GetMoviePlayer()->StopMovie();
}

FStreamableManager& UResourceManager::GetStreamMgr()
{
	return m_StreamableManager;
}

void UResourceManager::OnAssetLoaded(UObject* Asset)
{
	UE_LOG(ResourceManagerLog, Warning, TEXT("AssetLoades %s"), *Asset->GetName());
}

void UResourceManager::OnAssetAdded(const FAssetData& Asset)
{
	UE_LOG(ResourceManagerLog, Warning, TEXT("OnAssetAdded %s"), *Asset.AssetName.ToString());
	// 资源注册表新发现了一个资源
	//UE_LOG(ResourceManagerLog, Warning, TEXT("OnAssetAdded %s"), Asset.AssetName);
}