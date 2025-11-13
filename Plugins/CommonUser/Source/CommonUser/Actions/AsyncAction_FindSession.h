// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AsyncAction_Types.h"
#include "OnlineSessionSettings.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_FindSession.generated.h"

#define UE_API COMMONUSER_API


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFindSession_Delegate, const TArray<FString>&, Sessions);


UCLASS(MinimalAPI)
class UAsyncAction_FindSession : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AutoCreateRefTerm = "SessionSettings", WorldContext = "WorldContextObject"), Category="CommonUser")
	static UE_API UAsyncAction_FindSession* FindSession(
		UObject* WorldContextObject,
		APlayerController* Player,
		TMap<FName, FEIKAttribute> SessionSettings,
		int32 MaxResults = 10
		);

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FFindSession_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FFindSession_Delegate OnFailure;

	UE_API UAsyncAction_FindSession()
	{
		SessionSearch = MakeShared<FOnlineSessionSearch>();
	}

protected:
	FDelegateHandle FindSessionDelegateHandle;
	TWeakObjectPtr<UObject> WorldContextObject;
	TWeakObjectPtr<APlayerController> Player;
	TMap<FName, FEIKAttribute> SessionSettings;
	int32 MaxResults = 10;
	bool bIsCancelled = false;
	bool bIsSearching = false;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	UE_API virtual void Activate() override;
	UE_API void Execute_FindSession();
	UE_API void OnFindSessionCompleted(bool bWasSuccessful);
	UE_API virtual void Cancel() override;
};

#undef UE_API