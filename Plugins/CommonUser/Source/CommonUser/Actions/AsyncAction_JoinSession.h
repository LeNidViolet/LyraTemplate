// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AsyncAction_Types.h"
#include "Engine/CancellableAsyncAction.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "AsyncAction_JoinSession.generated.h"

#define UE_API COMMONUSER_API

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FJoinSession_Delegate);

UCLASS(MinimalAPI)
class UAsyncAction_JoinSession : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category="CommonUser")
	static UE_API UAsyncAction_JoinSession* JoinSession(
		UObject* WorldContextObject,
		APlayerController* Player,
		const FString& SessionId,
		TMap<FName, FEIKAttribute> MemberSettings
		);

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FJoinSession_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FJoinSession_Delegate OnFailure;


protected:
	TWeakObjectPtr<UObject> WorldContextObject;
	TWeakObjectPtr<APlayerController> Player;
	FString SessionId;
	TMap<FName, FEIKAttribute> MemberSettings;
	FDelegateHandle JoinSessionDelegateHandle;
	bool bIsCancelled = false;
	bool bIsSearching = false;
	bool bIsJoining = false;

	UE_API virtual void Activate() override;
	UE_API void Execute_JoinSession();
	UE_API void OnSingleSessionResultComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& SearchResult);
	UE_API void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	UE_API virtual void Cancel() override;
};

#undef UE_API
