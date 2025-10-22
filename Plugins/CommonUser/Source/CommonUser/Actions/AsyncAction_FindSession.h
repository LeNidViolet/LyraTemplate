// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AsyncAction_Types.h"
#include "OnlineSessionSettings.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_FindSession.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFindSession_Delegate, const TArray<FString>&, Sessions);


UCLASS(MinimalAPI)
class UAsyncAction_FindSession : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", AutoCreateRefTerm = "SessionSettings", WorldContext = "WorldContextObject"), Category="CommonUser")
	static COMMONUSER_API UAsyncAction_FindSession* FindSession(
		UObject* WorldContextObject,
		APlayerController* Player,
		TMap<FName, FEIKAttribute> SessionSettings,
		int32 MaxResults = 10
		);

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FFindSession_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FFindSession_Delegate OnFailure;

	UAsyncAction_FindSession()
	{
		SessionSearch = MakeShared<FOnlineSessionSearch>();
	}

protected:
	FDelegateHandle FindSessionDelegateHandle;
	TWeakObjectPtr<UObject> WorldContextObject;
	TWeakObjectPtr<APlayerController> Player;
	TMap<FName, FEIKAttribute> SessionSettings;
	int32 MaxResults = 10;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	virtual void Activate() override;
	void Execute_FindSessioin();
	void OnFindSessionCompleted(bool bWasSuccessful);
};
