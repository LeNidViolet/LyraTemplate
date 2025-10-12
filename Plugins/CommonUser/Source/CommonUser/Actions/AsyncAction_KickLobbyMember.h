// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "AsyncAction_KickLobbyMember.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FKickLobbyMember_Delegate);


UCLASS(MinimalAPI)
class UAsyncAction_KickLobbyMember : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category="CommonUser")
	static COMMONUSER_API UAsyncAction_KickLobbyMember* KickLobbyMember(
		UObject* WorldContextObject,
		APlayerController* Player,
		FUniqueNetIdRepl TargetUserUniqueId
		);

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FKickLobbyMember_Delegate OnSuccess;

protected:
	TWeakObjectPtr<UObject> WorldContextObject;
	TWeakObjectPtr<APlayerController> Player;
	FUniqueNetIdRepl TargetUserUniqueId;

	virtual void Activate() override;
	void Execute_KickLobbyMember();
};
