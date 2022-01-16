#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FirstAsyncTask.generated.h"

UCLASS()
class EVERYTHIN_API AFirstAsyncTask : public AActor
{
	GENERATED_BODY()

public:
	AFirstAsyncTask();

	UFUNCTION(BlueprintCallable)
		void AsyncLoadTextFile(const FString& FilePath);

	UFUNCTION(BlueprintImplementableEvent)
		void OnFileLoaded(const FString& FileContent);

};
