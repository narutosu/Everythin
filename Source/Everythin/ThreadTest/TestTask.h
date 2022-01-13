#pragma once
#include "CoreMinimal.h"

class EVERYTHIN_API FTestTask :public FNonAbandonableTask
{
public:
	FTestTask(int32 prime_count);
	~FTestTask();

	int32 prime_count;
public:

	FORCENOINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FTestTask,STATGROUP_ThreadPoolAsyncTasks)
	}

	void DoWork();
	void DoWorkMain();
};