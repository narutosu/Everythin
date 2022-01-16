#pragma once
#include "CoreMinimal.h"

class EVERYTHIN_API FTestTask :public FNonAbandonableTask
{
	friend class FAutoDeleteAsyncTask<FTestTask>;
	FTestTask(int32 prime_count);
	~FTestTask();

	int32 prime_count;

	FORCENOINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FTestTask,STATGROUP_ThreadPoolAsyncTasks)
	}

	void DoWork();
	void DoWorkMain();
};