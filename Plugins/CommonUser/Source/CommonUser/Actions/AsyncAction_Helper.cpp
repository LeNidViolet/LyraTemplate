
#include "AsyncAction_Helper.h"

#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "CommonUserSubsystem.h"

bool AsyncAction_Helper::UpdateMemberAttributes(
	UObject* WorldContextObject,
	int32 LocalUserNum,
	const FName& SessionName,
	const TMap<FName, FEIKAttribute>& MemberSettings)
{
	bool Result = false;

	UGameInstance* GameInstance = WorldContextObject->GetWorld()->GetGameInstance();
	UCommonUserSubsystem* CommonUserSubsystem = GameInstance->GetSubsystem<UCommonUserSubsystem>();
	check(CommonUserSubsystem);
	const UCommonUserInfo* LocalUserInfo = CommonUserSubsystem->GetUserInfoForLocalPlayerIndex(LocalUserNum);
	if (LocalUserInfo)
	{
		if (const IOnlineSubsystem *OnlineSubsystem = Online::GetSubsystem(WorldContextObject->GetWorld()))
		{
			if(const IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				FNamedOnlineSession* NamedOnlineSession = SessionPtr->GetNamedSession(SessionName);
				if (NamedOnlineSession)
				{
					FUniqueNetIdPtr LocalUserUniqueNetId = LocalUserInfo->GetNetId().GetV1();
					if (LocalUserUniqueNetId.IsValid())
					{
						FString LocalUserEpicId = GetEpicID(LocalUserUniqueNetId);
						FString LocalUserProductId = GetProductUserID(LocalUserUniqueNetId);

						for (auto & MemberPair : NamedOnlineSession->SessionSettings.MemberSettings)
						{
							FUniqueNetIdPtr TargetUserUniqueNetId = MemberPair.Key;
							FString TargetUserEpicId = GetEpicID(TargetUserUniqueNetId);
							FString TargetUserProductId = GetProductUserID(TargetUserUniqueNetId);

							if (LocalUserEpicId == TargetUserEpicId && LocalUserProductId == TargetUserProductId)
							{
								// Update member settings
								// Note: This will overwrite existing settings with the same key

								for (const TPair<FName, FEIKAttribute>& AttributePair : MemberSettings)
								{
									const FName KeyName = AttributePair.Key;
									const FVariantData Variant = AttributePair.Value.ToVariantData();

									FOnlineSessionSetting Setting(Variant, EOnlineDataAdvertisementType::ViaOnlineService);

									MemberPair.Value.Add(KeyName, Setting);
								}
								Result = SessionPtr->UpdateSession(SessionName, NamedOnlineSession->SessionSettings, true);
								break;
							}
						}
					}
				}
			}
		}
	}

	return Result;
}



FString AsyncAction_Helper::GetProductUserID(FUniqueNetIdPtr& UniqueNetId)
{
	if(UniqueNetId.IsValid())
	{
		const FString String_UserID = UniqueNetId.Get()->ToString();
		TArray<FString> Substrings;
		String_UserID.ParseIntoArray(Substrings, TEXT("|"));
		// Check if the split was successful
		if (Substrings.Num() == 2)
		{
			return Substrings[1];
		}
		if(Substrings.Num() == 1)
		{
			return Substrings[0];
		}
	}

	return FString();
}

FString AsyncAction_Helper::GetEpicID(FUniqueNetIdPtr& UniqueNetId)
{
	if(UniqueNetId.IsValid())
	{
		const FString String_UserID = UniqueNetId.Get()->ToString();
		TArray<FString> Substrings;
		String_UserID.ParseIntoArray(Substrings, TEXT("|"));
		// Check if the split was successful
		if (Substrings.Num() == 2)
		{
			return Substrings[0];
		}
	}

	return FString();
}
