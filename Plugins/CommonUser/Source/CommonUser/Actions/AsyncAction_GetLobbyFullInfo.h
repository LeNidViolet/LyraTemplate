// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AsyncAction_Types.h"
#include "Engine/CancellableAsyncAction.h"
#include "AsyncAction_GetLobbyFullInfo.generated.h"


#define UE_API COMMONUSER_API


USTRUCT (BlueprintType)
struct FLobbyMemberFullInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString EpicId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ProductId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsOwner = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FUniqueNetIdRepl NetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FEIKAttribute> Attributes;
};

USTRUCT(BlueprintType)
struct FLobbyFullInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString LobbyId;

	/** The number of publicly available connections that are available (read only) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumOpenPublicConnections = 0;

	/** The number of publicly available connections advertised */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumPublicConnections = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OwnerEpicId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString OwnerProductId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FUniqueNetIdRepl OwnerNetId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, FEIKAttribute> Attributes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FLobbyMemberFullInfo> Members;
};




DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGetLobbyFullInfo_Delegate, FLobbyFullInfo, LobbyFullInfo);




UCLASS(MinimalAPI)
class UAsyncAction_GetLobbyFullInfo : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject"), Category="CommonUser")
	static UE_API UAsyncAction_GetLobbyFullInfo* GetLobbyFullInfo(
		UObject* WorldContextObject,
		APlayerController* Player);

	UPROPERTY(BlueprintAssignable, DisplayName="Success")
	FGetLobbyFullInfo_Delegate OnSuccess;
	UPROPERTY(BlueprintAssignable, DisplayName="Failure")
	FGetLobbyFullInfo_Delegate OnFailure;

protected:
	TWeakObjectPtr<UObject> WorldContextObject;
	TWeakObjectPtr<APlayerController> Player;
	FLobbyFullInfo Result;
	bool bIsCancelled = false;
	bool bIsSearching = false;

	UE_API virtual void Activate() override;
	UE_API void Execute_GetLobbyFullInfo();
	UE_API virtual void Cancel() override;
};

#undef UE_API
