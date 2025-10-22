// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CommonSessionSubsystemOssv1.generated.h"




DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSessionParticipantJoined_C, FName /*SessionName*/, const FUniqueNetIdRepl& /*UniqueId*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSessionParticipantJoined_Dynamic, FName, SessionName, const FUniqueNetIdRepl&, UniqueId);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSessionParticipantLeft_C, FName /*SessionName*/, const FUniqueNetIdRepl& /*UniqueId*/);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSessionParticipantLeft_Dynamic, FName, SessionName, const FUniqueNetIdRepl&, UniqueId);


UCLASS(MinimalAPI, BlueprintType, Config=Engine)
class UCommonSessionSubsystemOssv1 : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	UCommonSessionSubsystemOssv1();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	UFUNCTION(BlueprintPure, Category = "Session")
	COMMONUSER_API FName GetLobbyName() const { return FName(TEXT("GameLobby_Ossv1")); }

	UFUNCTION(BlueprintPure, Category = "Session")
	COMMONUSER_API FName GetSessionName() const { return FName(TEXT("GameSession_Ossv1")); }

	UFUNCTION(BlueprintPure, Category = "Session")
	COMMONUSER_API bool IsLobbyName(FName Name) const { return Name == GetLobbyName(); }

	UFUNCTION(BlueprintPure, Category = "Session")
	COMMONUSER_API bool IsSessionName(FName Name) const { return Name == GetSessionName(); }

	UFUNCTION(BlueprintPure, Category = "Session")
	COMMONUSER_API bool IsLocalUserNetId(const FUniqueNetIdRepl& NetId) const;

	UFUNCTION(BlueprintPure, Category = "Session")
	COMMONUSER_API bool IsSessionOwnerNetId(FName SessionName, const FUniqueNetIdRepl& NetId) const;

	// EVENTs
	FOnSessionParticipantJoined_C OnSessionParticipantJoinedEvent;
	UPROPERTY(BlueprintAssignable, Category = "Session", meta = (DisplayName = "On Participant Joined"))
	FOnSessionParticipantJoined_Dynamic K2_OnSessionParticipantJoinedEvent;

	FOnSessionParticipantLeft_C OnSessionParticipantLeftEvent;
	UPROPERTY(BlueprintAssignable, Category = "Session", meta = (DisplayName = "On Participant Left"))
	FOnSessionParticipantLeft_Dynamic K2_OnSessionParticipantLeftEvent;

	UFUNCTION(BlueprintCallable, Category = "Networking|Debug")
	COMMONUSER_API void LogNetEnvironment(AActor* Actor, APlayerController* PlayerController = nullptr);

protected:
	void BindOnlineDelegates();
	void UnbindOnlineDelegates();

	FDelegateHandle OnLoginCompleteDelegate;
	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	FDelegateHandle OnLogoutCompleteDelegate;
	void OnLogoutComplete(int32 LocalUserNum, bool bWasSuccessful);
	FDelegateHandle OnLoginStatusChangeDelegate;
	void OnLoginStatusChange(int32 LocalUserNum, ELoginStatus::Type PreviousStatus, ELoginStatus::Type NewStatus, const FUniqueNetId& UserId);


	UFUNCTION()
	void OnUserPrivilegeChanged(const UCommonUserInfo* UserInfo, ECommonUserPrivilege Privilege, ECommonUserAvailability OldAvailability, ECommonUserAvailability NewAvailability);
	UFUNCTION()
	void OnUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error, ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext);



	FDelegateHandle OnCreateSessionCompleteDelegate;
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	FDelegateHandle OnStartSessionCompleteDelegate;
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);
	FDelegateHandle OnEndSessionCompleteDelegate;
	void OnEndSessionComplete(FName SessionName, bool bWasSuccessful);
	FDelegateHandle OnDestroySessionCompleteDelegate;
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	FDelegateHandle OnJoinSessionCompleteDelegate;
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	FDelegateHandle OnSessionParticipantJoinedDelegate;
	void OnSessionParticipantJoined(FName SessionName, const FUniqueNetId& UniqueId);
	FDelegateHandle OnSessionParticipantLeftDelegate;
	void OnSessionParticipantLeft(FName SessionName, const FUniqueNetId& UniqueId, EOnSessionParticipantLeftReason Reason);
	FDelegateHandle OnSessionFailtureDelegate;
	void OnSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType);

	FDelegateHandle OnNetworkFailureDelegate;
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);
	FDelegateHandle OnTravelFailureDelegate;
	void OnTravelFailure(UWorld* World, ETravelFailure::Type FailureType, const FString& ReasonString);

	void NotifySessionParticipantJoined(FName SessionName, const FUniqueNetId& UniqueId);
	void NotifySessionParticipantLeft(FName SessionName, const FUniqueNetId& UniqueId);

private:

	/** True if this is a dedicated server, which doesn't require a LocalPlayer to create a session */
	bool bIsDedicatedServer = false;
};
