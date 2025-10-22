// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncAction_LeaveSession.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "AsyncAction_LogChannel.h"


UAsyncAction_LeaveSession* UAsyncAction_LeaveSession::LeaveSession(UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		UE_LOG(LogCommonSessionAsyncAction, Error, TEXT("UAsyncAction_LeaveSession::LeaveSession: Invalid parameters"));
		return nullptr;
	}
	UAsyncAction_LeaveSession* Action = NewObject<UAsyncAction_LeaveSession>();
	Action->WorldContextObject = WorldContextObject;
	Action->RegisterWithGameInstance(WorldContextObject);
	return Action;
}

void UAsyncAction_LeaveSession::Activate()
{
	Execute_LeaveSession();
	Super::Activate();
}

void UAsyncAction_LeaveSession::Execute_LeaveSession()
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
