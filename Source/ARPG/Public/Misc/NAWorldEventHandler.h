// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "NAWorldEventHandler.generated.h"

/**
 * 
 */


class ANAPlaceableItemActor;


UCLASS()
class ARPG_API UNAWorldEventHandler : public UObject
{
	GENERATED_BODY()

public:
	FORCEINLINE static UNAWorldEventHandler* GetInstance()
	{
		if (!Instance)
		{
			Instance = NewObject<UNAWorldEventHandler>();
			Instance->AddToRoot();
		}
		return Instance;
	}

	void RegisterEvent(FString EventName, TFunction<void()> Delegate)
	{
		Events.Add(EventName, Delegate);
	}
	
	void TriggerEvent(FString Name)
	{
		TArray<TFunction<void()>> Delegates;
		Events.MultiFind(Name, Delegates);

		if (!Delegates.IsEmpty())
		{
			for (TFunction<void()> Delegate : Delegates)
				Delegate();
		}
	}

	void UnRegisterEvent(FString Name)
	{
		Events.Remove(Name);
	}

	
	
private:
	TMultiMap<FString, TFunction<void()>> Events;
	
private:
	static UNAWorldEventHandler* Instance;
};

UNAWorldEventHandler* UNAWorldEventHandler::Instance = nullptr;
