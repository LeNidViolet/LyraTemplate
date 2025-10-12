// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AsyncAction_Types.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_UpdateLobbyMemberAttribute.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUpdateLobbyMemberAttribute_Delegate);


UCLASS(MinimalAPI)
class UAsyncAction_UpdateLobbyMemberAttribute : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category="CommonUser")
	static COMMONUSER_API UAsyncAction_UpdateLobbyMemberAttribute* UpdateLobbyMemberAttribute(
		UObject* WorldContextObject,
		APlayerController* Player,
		TMap<FName, FEIKAttribute> MemberSettings
		);

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FUpdateLobbyMemberAttribute_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FUpdateLobbyMemberAttribute_Delegate OnFailure;

protected:
	TWeakObjectPtr<UObject> WorldContextObject;
	TWeakObjectPtr<APlayerController> Player;
	TMap<FName, FEIKAttribute> MemberSettings;

	virtual void Activate() override;
	void Execute_UpdateLobbyMemberAttribute();
};
