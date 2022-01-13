#include "TestTask.h"

FTestTask::FTestTask(int32 _prime_count)
{
	prime_count = _prime_count;
}

FTestTask::~FTestTask()
{

}

void FTestTask::DoWork()
{
	int primes_found = 0;
	int current_Test_number = 2;
	while (primes_found < prime_count) {
		bool is_prime = true;
		for (int i = 2; i < current_Test_number / 2; i++) {
			if (current_Test_number % i == 0)
			{
				is_prime = false;
				break;
			}
		}
		if (is_prime) {
			primes_found++;
			if (primes_found % 1000 == 0)
			{
				UE_LOG(LogTemp, Warning, TEXT("prime found:%i"), primes_found);
			}
		}
		current_Test_number++;
	}
	UE_LOG(LogTemp, Warning, TEXT("work over"));
}

void FTestTask::DoWorkMain()
{
	DoWork();
}
