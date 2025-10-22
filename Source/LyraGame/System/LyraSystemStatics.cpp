// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraSystemStatics.h"
#include "Engine/AssetManagerTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Engine/AssetManager.h"
#include "LyraLogChannels.h"
#include "Components/MeshComponent.h"
#include "GameModes/LyraUserFacingExperienceDefinition.h"
#include "Kismet/GameplayStatics.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(LyraSystemStatics)

TSoftObjectPtr<UObject> ULyraSystemStatics::GetTypedSoftObjectReferenceFromPrimaryAssetId(FPrimaryAssetId PrimaryAssetId, TSubclassOf<UObject> ExpectedAssetType)
{
	if (UAssetManager* Manager = UAssetManager::GetIfInitialized())
	{
		FPrimaryAssetTypeInfo Info;
		if (Manager->GetPrimaryAssetTypeInfo(PrimaryAssetId.PrimaryAssetType, Info) && !Info.bHasBlueprintClasses)
		{
			if (UClass* AssetClass = Info.AssetBaseClassLoaded)
			{
				if ((ExpectedAssetType == nullptr) || !AssetClass->IsChildOf(ExpectedAssetType))
				{
					return nullptr;
				}
			}
			else
			{
				UE_LOG(LogLyra, Warning, TEXT("GetTypedSoftObjectReferenceFromPrimaryAssetId(%s, %s) - AssetBaseClassLoaded was unset so we couldn't validate it, returning null"),
					*PrimaryAssetId.ToString(),
					*GetPathNameSafe(*ExpectedAssetType));
			}

			return TSoftObjectPtr<UObject>(Manager->GetPrimaryAssetPath(PrimaryAssetId));
		}
	}
	return nullptr;
}

FPrimaryAssetId ULyraSystemStatics::GetPrimaryAssetIdFromUserFacingExperienceName(const FString& AdvertisedExperienceID)
{
	const FPrimaryAssetType Type(ULyraUserFacingExperienceDefinition::StaticClass()->GetFName());
	return FPrimaryAssetId(Type, FName(*AdvertisedExperienceID));
}

void ULyraSystemStatics::PlayNextGame(const UObject* WorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World == nullptr)
	{
		return;
	}

	const FWorldContext& WorldContext = GEngine->GetWorldContextFromWorldChecked(World);
	FURL LastURL = WorldContext.LastURL;

#if WITH_EDITOR
	// To transition during PIE we need to strip the PIE prefix from maps.
	LastURL.Map = UWorld::StripPIEPrefixFromPackageName(LastURL.Map, WorldContext.World()->StreamingLevelsPrefix);
#endif

	// Add seamless travel option as we want to keep clients connected. This will fall back to hard travel if seamless is disabled
	LastURL.AddOption(TEXT("SeamlessTravel"));

	FString URL = LastURL.ToString();
	// If we don't remove the host/port info the server travel will fail.
	URL.RemoveFromStart(LastURL.GetHostPortString());
	
	const bool bAbsolute = false; // we want to use TRAVEL_Relative
	const bool bShouldSkipGameNotify = false;
	World->ServerTravel(URL, bAbsolute, bShouldSkipGameNotify);
}

void ULyraSystemStatics::SetScalarParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const float ParameterValue, bool bIncludeChildActors)
{
	if (TargetActor != nullptr)
	{
		TargetActor->ForEachComponent<UMeshComponent>(bIncludeChildActors, [=](UMeshComponent* InComponent)
		{
			InComponent->SetScalarParameterValueOnMaterials(ParameterName, ParameterValue);
		});
	}
}

void ULyraSystemStatics::SetVectorParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const FVector ParameterValue, bool bIncludeChildActors)
{
	if (TargetActor != nullptr)
	{
		TargetActor->ForEachComponent<UMeshComponent>(bIncludeChildActors, [=](UMeshComponent* InComponent)
		{
			InComponent->SetVectorParameterValueOnMaterials(ParameterName, ParameterValue);
		});
	}
}

void ULyraSystemStatics::SetColorParameterValueOnAllMeshComponents(AActor* TargetActor, const FName ParameterName, const FLinearColor ParameterValue, bool bIncludeChildActors)
{
	SetVectorParameterValueOnAllMeshComponents(TargetActor, ParameterName, FVector(ParameterValue), bIncludeChildActors);
}

TArray<UActorComponent*> ULyraSystemStatics::FindComponentsByClass(AActor* TargetActor, TSubclassOf<UActorComponent> ComponentClass, bool bIncludeChildActors)
{
	TArray<UActorComponent*> Components;
	if (TargetActor != nullptr)
	{
		TargetActor->GetComponents(ComponentClass, /*out*/ Components, bIncludeChildActors);

	}
	return MoveTemp(Components);
}

bool ULyraSystemStatics::TravelExperience(
	const UObject* WorldContextObject,
	ULyraUserFacingExperienceDefinition* FacingExperience,
	TMap<FString, FString> ExtraArgs, bool bAbsolute, ECommonSessionOnlineMode OnlineMode)
{
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World == nullptr)
	{
		return false;
	}

	FAssetData MapAssetData;
	bool bOK = UAssetManager::Get().GetPrimaryAssetData(FacingExperience->MapID, MapAssetData);
	check(bOK);

	FString TravelURL = MapAssetData.PackageName.ToString();
	switch (OnlineMode)
	{
	case ECommonSessionOnlineMode::Offline:
		break;
	case ECommonSessionOnlineMode::Online:
		TravelURL += TEXT("?listen");
		break;
	case ECommonSessionOnlineMode::LAN:
		TravelURL += TEXT("?bIsLanMatch?listen");
		break;
	}

	for (const auto& Kvp : ExtraArgs)
	{
		if (!Kvp.Key.IsEmpty())
		{
			if (Kvp.Value.IsEmpty())
			{
				TravelURL += FString::Printf(TEXT("?%s"), *Kvp.Key);
			}
			else
			{
				TravelURL += FString::Printf(TEXT("?%s=%s"), *Kvp.Key, *Kvp.Value);
			}
		}
	}

	for (const auto& Kvp : FacingExperience->ExtraArgs)
	{
		if (!Kvp.Key.IsEmpty())
		{
			if (Kvp.Value.IsEmpty())
			{
				TravelURL += FString::Printf(TEXT("?%s"), *Kvp.Key);
			}
			else
			{
				TravelURL += FString::Printf(TEXT("?%s=%s"), *Kvp.Key, *Kvp.Value);
			}
		}
	}

	bool bExists = UGameplayStatics::HasOption(TravelURL, "Experience");
	if (!bExists)
	{
		FName ExperienceName = FacingExperience->ExperienceID.PrimaryAssetName;
		TravelURL += FString::Printf(TEXT("?Experience=%s"), *ExperienceName.ToString());
	}

	UE_LOG(LogLyra, Log, TEXT("Traveling to experience via URL %s"), *TravelURL);
	return World->ServerTravel(TravelURL, bAbsolute);
}

bool ULyraSystemStatics::TravelExperienceWithName(
	const UObject* WorldContextObject,
	FName FacingExperience, TMap<FString, FString> ExtraArgs,
	bool bAbsolute, ECommonSessionOnlineMode OnlineMode)
{
	UAssetManager* AssetManager = UAssetManager::GetIfInitialized();
	ensure(AssetManager);
	FPrimaryAssetId AssetId(TEXT("LyraUserFacingExperienceDefinition"), FacingExperience);
	FSoftObjectPath AssetPath = AssetManager->GetPrimaryAssetPath(AssetId);
	if (AssetPath.IsValid())
	{
		UObject* Asset = AssetManager->GetStreamableManager().LoadSynchronous(AssetPath, true);
		if (ULyraUserFacingExperienceDefinition* FacingExperienceObject = Cast<ULyraUserFacingExperienceDefinition>(Asset))
		{
			return TravelExperience(WorldContextObject, FacingExperienceObject, ExtraArgs, bAbsolute, OnlineMode);
		}
	}
	return false;
}

