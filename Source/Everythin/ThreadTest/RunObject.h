#pragma once
#include "CoreMinimal.h"

class EVERYTHIN_API RunObject :public FRunnable
{
public:
	RunObject() {};
	~RunObject() {};

	virtual bool Init() override {
		bIsRunning = true;
		UE_LOG(LogTemp, Display, TEXT("RunObject Init"));
		return true;
	}

	virtual uint32 Run() override {
		while (bIsRunning) {
			UE_LOG(LogTemp, Display, TEXT("RunObject Run %d"), runTimes);
			++runTimes;
			FPlatformProcess::Sleep(1);
		}
		return 0;
	}

	virtual void Stop() override {
		UE_LOG(LogTemp, Display, TEXT("RunObject Stop"));
		bIsRunning = false;
	}

	virtual void Exit() override {
		UE_LOG(LogTemp, Display, TEXT("RunObject Exit"));
	}

private:
	int runTimes = 0;
	bool bIsRunning = false;
};