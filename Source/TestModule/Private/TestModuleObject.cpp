// Fill out your copyright notice in the Description page of Project Settings.

#include "TestModuleObject.h"

UTestModuleObject::UTestModuleObject()
{
	
}

void UTestModuleObject::Fire()
{
	UE_LOG(TestModuleLog, Warning, TEXT("UTestModuleObject fire"));
}
