// Fill out your copyright notice in the Description page of Project Settings.


#include "CommonSessionSubsystemOssv1.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Interfaces/OnlineIdentityInterface.h"

DECLARE_LOG_CATEGORY_EXTERN(LogCommonSessionOssv1, Log, All);
DEFINE_LOG_CATEGORY(LogCommonSessionOssv1);

UCommonSessionSubsystemOssv1::UCommonSessionSubsystemOssv1()
{
}

void UCommonSessionSubsystemOssv1::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	BindOnlineDelegates();
}

void UCommonSessionSubsystemOssv1::Deinitialize()
{
	UnbindOnlineDelegates();

	Super::Deinitialize();
}

bool UCommonSessionSubsystemOssv1::ShouldCreateSubsystem(UObject* Outer) const
{
	TArray<UClass*> ChildClasses;
	GetDerivedClasses(GetClass(), ChildClasses, false);

	// Only create an instance if there is not a game-specific subclass
	return ChildClasses.Num() == 0;
}

bool UCommonSessionSubsystemOssv1::IsLocalUserNetId(const FUniqueNetIdRepl& NetId) const
{
	if (NetId.IsValid())
	{
		UCommonUserSubsystem* UserSubsystem = GetGameInstance()->GetSubsystem<UCommonUserSubsystem>();
		if (UserSubsystem)
		{
			for (int32 i = 0; i < UserSubsystem->GetMaxLocalPlayers(); i++)
			{
				const UCommonUserInfo* UserInfo = UserSubsystem->GetUserInfoForLocalPlayerIndex(i);
				if (UserInfo)
				{
					FUniqueNetIdRepl UserNetId = UserInfo->GetNetId();
					if (UserNetId.IsValid() && *UserNetId == *NetId)
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool UCommonSessionSubsystemOssv1::IsSessionOwnerNetId(FName SessionName, const FUniqueNetIdRepl& NetId) const
{
	if (!SessionName.IsNone() && NetId.IsValid())
	{
		IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
		if (OnlineSub)
		{
			const IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
			if (SessionInterface)
			{
				FNamedOnlineSession* NamedOnlineSession = SessionInterface->GetNamedSession(SessionName);
				if (NamedOnlineSession)
				{
					if (NamedOnlineSession->OwningUserId.IsValid())
					{
						return *NamedOnlineSession->OwningUserId == *NetId;
					}
				}
			}
		}
	}

	return false;
}

void UCommonSessionSubsystemOssv1::BindOnlineDelegates()
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		const IOnlineIdentityPtr IdentityInterface = OnlineSub->GetIdentityInterface();
		if (IdentityInterface)
		{
			OnLoginCompleteDelegate = IdentityInterface->OnLoginCompleteDelegates->AddUObject(
				this, &ThisClass::OnLoginComplete);
			OnLogoutCompleteDelegate = IdentityInterface->OnLogoutCompleteDelegates->AddUObject(
				this, &ThisClass::OnLogoutComplete);
			OnLoginStatusChangeDelegate = IdentityInterface->OnLoginStatusChangedDelegates->AddUObject(
				this, &ThisClass::OnLoginStatusChange);
		}

		const IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface)
		{
			OnCreateSessionCompleteDelegate = SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(
				this, &ThisClass::OnCreateSessionComplete);
			OnStartSessionCompleteDelegate = SessionInterface->OnStartSessionCompleteDelegates.AddUObject(
				this, &ThisClass::OnStartSessionComplete);
			OnEndSessionCompleteDelegate = SessionInterface->OnEndSessionCompleteDelegates.AddUObject(
				this, &ThisClass::OnEndSessionComplete);
			OnDestroySessionCompleteDelegate = SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(
				this, &ThisClass::OnDestroySessionComplete);
			OnJoinSessionCompleteDelegate = SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(
				this, &ThisClass::OnJoinSessionComplete);
			OnSessionParticipantJoinedDelegate = SessionInterface->OnSessionParticipantJoinedDelegates.AddUObject(
				this, &ThisClass::OnSessionParticipantJoined);
			OnSessionParticipantLeftDelegate = SessionInterface->OnSessionParticipantLeftDelegates.AddUObject(
				this, &ThisClass::OnSessionParticipantLeft);
			OnSessionFailtureDelegate = SessionInterface->OnSessionFailureDelegates.AddUObject(
				this, &ThisClass::OnSessionFailure);
		}

		UCommonUserSubsystem* CommonUserSubsystem = GetGameInstance()->GetSubsystem<UCommonUserSubsystem>();
		if (CommonUserSubsystem)
		{
			CommonUserSubsystem->OnUserPrivilegeChanged.AddDynamic(
				this, &ThisClass::OnUserPrivilegeChanged);

			CommonUserSubsystem->OnUserInitializeComplete.AddDynamic(
				this, &ThisClass::OnUserInitialized);
		}
	}

	OnNetworkFailureDelegate = GEngine->OnNetworkFailure().AddUObject(
		this, &ThisClass::OnNetworkFailure);
	OnTravelFailureDelegate = GEngine->OnTravelFailure().AddUObject(
		this, &ThisClass::OnTravelFailure);
}

void UCommonSessionSubsystemOssv1::UnbindOnlineDelegates()
{
	IOnlineSubsystem* OnlineSub = Online::GetSubsystem(GetWorld());
	if (OnlineSub)
	{
		const IOnlineIdentityPtr IdentityInterface = OnlineSub->GetIdentityInterface();
		if (IdentityInterface)
		{
			IdentityInterface->OnLoginCompleteDelegates->Remove(OnLoginCompleteDelegate);
			IdentityInterface->OnLogoutCompleteDelegates->Remove(OnLogoutCompleteDelegate);
			IdentityInterface->OnLoginStatusChangedDelegates->Remove(OnLoginStatusChangeDelegate);
		}

		const IOnlineSessionPtr SessionInterface = OnlineSub->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.Remove(OnCreateSessionCompleteDelegate);
			SessionInterface->OnStartSessionCompleteDelegates.Remove(OnStartSessionCompleteDelegate);
			SessionInterface->OnEndSessionCompleteDelegates.Remove(OnEndSessionCompleteDelegate);
			SessionInterface->OnDestroySessionCompleteDelegates.Remove(OnDestroySessionCompleteDelegate);
			SessionInterface->OnJoinSessionCompleteDelegates.Remove(OnJoinSessionCompleteDelegate);
			SessionInterface->OnSessionParticipantJoinedDelegates.Remove(OnSessionParticipantJoinedDelegate);
			SessionInterface->OnSessionParticipantLeftDelegates.Remove(OnSessionParticipantLeftDelegate);
			SessionInterface->OnSessionFailureDelegates.Remove(OnSessionFailtureDelegate);
		}

		UCommonUserSubsystem* CommonUserSubsystem = GetGameInstance()->GetSubsystem<UCommonUserSubsystem>();
		if (CommonUserSubsystem)
		{
			CommonUserSubsystem->OnUserPrivilegeChanged.RemoveDynamic(
				this, &ThisClass::OnUserPrivilegeChanged);

			CommonUserSubsystem->OnUserInitializeComplete.RemoveDynamic(
				this, &ThisClass::OnUserInitialized);
		}
	}

	GEngine->OnNetworkFailure().Remove(OnNetworkFailureDelegate);
}

void UCommonSessionSubsystemOssv1::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId,
	const FString& Error)
{
	UE_LOG(LogCommonSessionOssv1, Log,
		TEXT("OnUserLoginCompleted(LocalUserNum=%d bWasSuccessful=%d UserId=%s Error=%s)"),
		LocalUserNum, bWasSuccessful, *UserId.ToString(), *Error);
}

void UCommonSessionSubsystemOssv1::OnLogoutComplete(int32 LocalUserNum, bool bWasSuccessful)
{
	UE_LOG(LogCommonSessionOssv1, Log,
		TEXT("OnUserLogoutCompleted(LocalUserNum=%d bWasSuccessful=%d)"),
		LocalUserNum, bWasSuccessful);
}

void UCommonSessionSubsystemOssv1::OnLoginStatusChange(int32 LocalUserNum, ELoginStatus::Type PreviousStatus,
	ELoginStatus::Type NewStatus, const FUniqueNetId& UserId)
{
	UE_LOG(LogCommonSessionOssv1, Log,
		TEXT("OnUserLoginStatusChanged(LocalUserNum=%d PreviousStatus=%s NewStatus=%s UserId=%s)"),
		LocalUserNum, ELoginStatus::ToString(PreviousStatus), ELoginStatus::ToString(NewStatus), *UserId.ToString());
}

void UCommonSessionSubsystemOssv1::OnUserPrivilegeChanged(const UCommonUserInfo* UserInfo,
	ECommonUserPrivilege Privilege, ECommonUserAvailability OldAvailability, ECommonUserAvailability NewAvailability)
{
	UE_LOG(LogCommonSessionOssv1, Log,
		TEXT("OnUserPrivilegeChanged(User: %s, Privilege: %s, OldAvailability: %s, NewAvailability: %s)"),
		*UserInfo->GetNickname(),
		LexToString(Privilege),
		LexToString(OldAvailability),
		LexToString(NewAvailability));
}

void UCommonSessionSubsystemOssv1::OnUserInitialized(const UCommonUserInfo* UserInfo, bool bSuccess, FText Error,
	ECommonUserPrivilege RequestedPrivilege, ECommonUserOnlineContext OnlineContext)
{
	UE_LOG(LogCommonSessionOssv1, Log,
		TEXT("OnUserInitialized(User: %s, bSuccess: %d, Error: %s, RequestedPrivilege: %s, OnlineContext: %s)"),
		*UserInfo->GetNickname(),
		bSuccess,
		*Error.ToString(),
		LexToString(RequestedPrivilege),
		LexToString(OnlineContext));
}


void UCommonSessionSubsystemOssv1::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogCommonSessionOssv1, Log,
		TEXT("OnCreateSessionComplete(SessionName: %s, bWasSuccessful: %d)"),
		*SessionName.ToString(), bWasSuccessful);
}

void UCommonSessionSubsystemOssv1::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogCommonSessionOssv1, Log,
		TEXT("OnStartSessionComplete(SessionName: %s, bWasSuccessful: %d)"),
		*SessionName.ToString(), bWasSuccessful);
}

void UCommonSessionSubsystemOssv1::OnEndSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogCommonSessionOssv1, Log,
		TEXT("OnEndSessionComplete(SessionName: %s, bWasSuccessful: %d)"),
		*SessionName.ToString(), bWasSuccessful);
}

void UCommonSessionSubsystemOssv1::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogCommonSessionOssv1, Log,
		TEXT("OnDestroySessionComplete(SessionName: %s, bWasSuccessful: %d)"),
		*SessionName.ToString(), bWasSuccessful);
}

void UCommonSessionSubsystemOssv1::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	const IOnlineSubsystem *OnlineSubsystem = Online::GetSubsystem(GetWorld());
	const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface();

	FString MapUrl;
	SessionPtr->GetResolvedConnectString(SessionName, MapUrl);

	UE_LOG(LogCommonSessionOssv1, Log,
		TEXT("OnJoinSessionComplete(SessionName: %s, Result: %s, MapUrl: %s)"),
		*SessionName.ToString(), LexToString(Result), *MapUrl);
}

void UCommonSessionSubsystemOssv1::OnSessionParticipantJoined(FName SessionName, const FUniqueNetId& UniqueId)
{
	UE_LOG(LogCommonSessionOssv1, Log,
		TEXT("OnSessionParticipantJoined(SessionName: %s, Participant: %s)"),
		*SessionName.ToString(), *UniqueId.ToString());
	NotifySessionParticipantJoined(SessionName, UniqueId);
}

void UCommonSessionSubsystemOssv1::OnSessionParticipantLeft(FName SessionName, const FUniqueNetId& UniqueId,
	EOnSessionParticipantLeftReason Reason)
{
	UE_LOG(LogCommonSessionOssv1, Log,
		TEXT("OnSessionParticipantLeft(SessionName: %s, Participant: %s, Reason: %s)"),
		*SessionName.ToString(), *UniqueId.ToString(), ToLogString(Reason));
	NotifySessionParticipantLeft(SessionName, UniqueId);
}

void UCommonSessionSubsystemOssv1::OnSessionFailure(const FUniqueNetId& NetId, ESessionFailure::Type FailureType)
{
	UE_LOG(LogCommonSessionOssv1, Error,
		TEXT("OnSessionFailure(NetId: %s, FailureType: %s)"),
		*NetId.ToString(), LexToString(FailureType));
}


void UCommonSessionSubsystemOssv1::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver,
	ENetworkFailure::Type FailureType, const FString& ErrorString)
{
	UE_LOG(LogCommonSessionOssv1, Error,
		TEXT("OnNetworkFailure(World: %s, NetDriver: %s, FailureType: %s, ErrorString: %s)"),
		*GetNameSafe(World),
		*GetNameSafe(NetDriver),
		ENetworkFailure::ToString(FailureType),
		*ErrorString);
}

void UCommonSessionSubsystemOssv1::OnTravelFailure(UWorld* World, ETravelFailure::Type FailureType,
	const FString& ReasonString)
{
	UE_LOG(LogCommonSessionOssv1, Error, TEXT("OnTravelFailure(World: %s, FailureType: %s, ReasonString: %s)"),
		*GetPathNameSafe(World),
		ETravelFailure::ToString(FailureType),
		*ReasonString);
}

void UCommonSessionSubsystemOssv1::NotifySessionParticipantJoined(FName SessionName, const FUniqueNetId& UniqueId)
{
	FUniqueNetIdRepl UniqueIdRepl(UniqueId);
	OnSessionParticipantJoinedEvent.Broadcast(SessionName, UniqueIdRepl);
	K2_OnSessionParticipantJoinedEvent.Broadcast(SessionName, UniqueIdRepl);
}

void UCommonSessionSubsystemOssv1::NotifySessionParticipantLeft(FName SessionName, const FUniqueNetId& UniqueId)
{
	FUniqueNetIdRepl UniqueIdRepl(UniqueId);
	OnSessionParticipantLeftEvent.Broadcast(SessionName, UniqueIdRepl);
	K2_OnSessionParticipantLeftEvent.Broadcast(SessionName, UniqueIdRepl);
}


FName GetNetModeName(ENetMode NetMode)
{
	switch (NetMode)
	{
	case ENetMode::NM_DedicatedServer: return FName(TEXT("NM_DedicatedServer"));
	case ENetMode::NM_Client: return FName(TEXT("NM_Client"));
	case ENetMode::NM_ListenServer: return FName(TEXT("NM_ListenServer"));
	case ENetMode::NM_Standalone: return FName(TEXT("NM_Standalone"));
	default: return FName(TEXT("Unknown"));
	}
}

void UCommonSessionSubsystemOssv1::LogNetEnvironment(AActor* Actor, APlayerController* PlayerController)
{

	UE_LOG(LogCommonSessionOssv1, Log, TEXT("LogNetEnvironment>>>>>>>>>>>>>>>> Actor[%s] PlayerController[%s]"),
		Actor ? *Actor->GetName() : TEXT("NONAME"),
		PlayerController ? *PlayerController->GetName() : TEXT("NONAME")
		);

	// Global
	UE_LOG(LogCommonSessionOssv1, Log, TEXT("LogNetEnvironment IsRunningDedicatedServer[%s]"), IsRunningDedicatedServer() ? TEXT("True") : TEXT("False"));
	UE_LOG(LogCommonSessionOssv1, Log, TEXT("LogNetEnvironment IsRunningClientOnly[%s]"), IsRunningClientOnly() ? TEXT("True") : TEXT("False"));
	UE_LOG(LogCommonSessionOssv1, Log, TEXT("LogNetEnvironment IsRunningGame[%s]"), IsRunningGame() ? TEXT("True") : TEXT("False"))

	// Net Mode
	UWorld* World = GetWorld();
	if (World)
	{
		ENetMode WorldNetMode = World->GetNetMode();
		UE_LOG(LogCommonSessionOssv1, Log, TEXT("LogNetEnvironment WorldNetMode[%s]"), *GetNetModeName(WorldNetMode).ToString());

	}
	else
	{
		UE_LOG(LogCommonSessionOssv1, Warning, TEXT("LogNetEnvironment GetWorld() Failed"));
	}

	if (Actor)
	{
		ENetMode ActorNetMode = Actor->GetNetMode();
		UE_LOG(LogCommonSessionOssv1, Log, TEXT("LogNetEnvironment Actor NetMode[%s]"), *GetNetModeName(ActorNetMode).ToString());

		// Role
		ENetRole LocalRole = Actor->GetLocalRole();
		ENetRole RemoteRole = Actor->GetRemoteRole();
		UE_LOG(LogCommonSessionOssv1, Log, TEXT("LogNetEnvironment Actor LocalRole[%s]"), *UEnum::GetValueAsString(LocalRole));
		UE_LOG(LogCommonSessionOssv1, Log, TEXT("LogNetEnvironment Actor RemoteRole[%s]"), *UEnum::GetValueAsString(RemoteRole));

		// Authority (GetLocalRole() == ROLE_Authority);
		UE_LOG(LogCommonSessionOssv1, Log, TEXT("LogNetEnvironment Actor Authority[%s]"), Actor->HasAuthority() ? TEXT("True") : TEXT("False"))
	}

	if (PlayerController)
	{
		UE_LOG(LogCommonSessionOssv1, Log, TEXT("LogNetEnvironment PlayerController IsLocalController[%s]"), PlayerController->IsLocalController() ? TEXT("True") : TEXT("False"))
		UE_LOG(LogCommonSessionOssv1, Log, TEXT("LogNetEnvironment PlayerController IsLocalPlayerController[%s]"), PlayerController->IsLocalPlayerController() ? TEXT("True") : TEXT("False"))
	}

	UE_LOG(LogCommonSessionOssv1, Log, TEXT("LogNetEnvironment<<<<<<<<<<<<<<<< Actor[%s] PlayerController[%s]"),
		Actor ? *Actor->GetName() : TEXT("NONAME"),
		PlayerController ? *PlayerController->GetName() : TEXT("NONAME")
		);
}