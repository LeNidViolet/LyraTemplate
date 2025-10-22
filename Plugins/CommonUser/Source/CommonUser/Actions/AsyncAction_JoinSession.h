// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AsyncAction_Types.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_JoinSession.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FJoinSession_Delegate);

UCLASS(MinimalAPI)
class UAsyncAction_JoinSession : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category="CommonUser")
	static COMMONUSER_API UAsyncAction_JoinSession* JoinSession(
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

	virtual void Activate() override;
	void Execute_JoinSession();
	void OnSingleSessionResultComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& SearchResult);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
};
