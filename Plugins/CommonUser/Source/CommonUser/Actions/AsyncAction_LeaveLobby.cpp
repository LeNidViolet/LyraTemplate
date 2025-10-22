// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction_LeaveLobby.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "AsyncAction_LogChannel.h"



UAsyncAction_LeaveLobby* UAsyncAction_LeaveLobby::LeaveLobby(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogCommonSessionAsyncAction, Error, TEXT("UAsyncAction_LeaveLobby::LeaveLobby: Invalid parameters"));
		return nullptr;
	}
	UAsyncAction_LeaveLobby* Action = NewObject<UAsyncAction_LeaveLobby>();
	Action->WorldContextObject = WorldContextObject;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UAsyncAction_LeaveLobby::Activate()
{
	Execute_LeaveLobby();
	Super::Activate();
}

void UAsyncAction_LeaveLobby::Execute_LeaveLobby()
{
	bool bSuccess = false;
	if (const IOnlineSubsystem *OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld()))
	{
		if(const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
		{
			SessionPtr->EndSession(NAME_GameSession);

			bSuccess = true;
		}
	}

	auto HandleResult = [this, bSuccess]()
	{
		if (bSuccess)
		{
			OnSuccess.Broadcast();
		}
		else
		{
			OnFailure.Broadcast();
		}

		SetReadyToDestroy();
	};

	UWorld* World = WorldContextObject->GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateLambda(HandleResult));
	}
	else
	{
		HandleResult();
	}
}