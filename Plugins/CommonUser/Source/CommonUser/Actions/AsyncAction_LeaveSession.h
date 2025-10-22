// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_LeaveSession.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLeaveSession_Delegate);

UCLASS(MinimalAPI)
class UAsyncAction_LeaveSession : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category="CommonUser")
	static COMMONUSER_API UAsyncAction_LeaveSession* LeaveSession(UObject* WorldContextObject);

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FLeaveSession_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FLeaveSession_Delegate OnFailure;

protected:

	FDelegateHandle DestroySessionDelegateHandle;
	TWeakObjectPtr<UObject> WorldContextObject;
	virtual void Activate() override;
	void Execute_LeaveSession();
};
