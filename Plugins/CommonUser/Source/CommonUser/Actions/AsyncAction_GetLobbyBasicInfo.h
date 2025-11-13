// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AsyncAction_Types.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_GetLobbyBasicInfo.generated.h"

#define UE_API COMMONUSER_API

USTRUCT(BlueprintType)
struct FLobbyBasicInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LobbyId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 PingInMs = 0;

	/** The number of publicly available connections that are available (read only) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumOpenPublicConnections = 0;

	/** The number of publicly available connections advertised */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumPublicConnections = 0;

	/** Owner of the session */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OwnerId;
	// FUniqueNetIdPtr OwningUserId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FEIKAttribute> Attributes;
};



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGetLobbyBasicInfo_Delegate, FLobbyBasicInfo, LobbyBasicInfo);





UCLASS(MinimalAPI)
class UAsyncAction_GetLobbyBasicInfo : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category="CommonUser")
	static UE_API UAsyncAction_GetLobbyBasicInfo* GetLobbyBasicInfo(
		UObject* WorldContextObject,
		APlayerController* Player,
		const FString& LobbyId,
		TArray<FName> ExposeAttributes
		);



	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FGetLobbyBasicInfo_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FGetLobbyBasicInfo_Delegate OnFailure;

protected:
	TWeakObjectPtr<UObject> WorldContextObject;
	TWeakObjectPtr<APlayerController> Player;
	TArray<FName> ExposeAttributes;
	FString LobbyId;
	bool bIsSearching = false;
	bool bIsCancelled = false;

	UE_API virtual void Activate() override;
	UE_API void Execute_GetLobbyBasicInfo();
	UE_API void OnSingleSessionResultComplete(int32 LocalUserNum, bool bWasSuccessful, const FOnlineSessionSearchResult& SearchResult);
	UE_API virtual void Cancel() override;
};

#undef UE_API
