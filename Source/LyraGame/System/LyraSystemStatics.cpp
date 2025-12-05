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
#include "NavigationSystem.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"

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

bool ULyraSystemStatics::FindValidSpawnLocationInCone(
	FVector& OutLocation,
	AActor* OriginActor,
	float MinRadius,
	float MaxRadius,
	float ConeHalfAngle)
{
	if (!OriginActor) return false;

	UWorld* World = OriginActor->GetWorld();
	if (!World) return false;

	// 1. 计算起点高度
    FVector OriginLocation = OriginActor->GetActorLocation();

    // 尝试获取胶囊体组件来精确计算
    if (UCapsuleComponent* Capsule = OriginActor->FindComponentByClass<UCapsuleComponent>())
    {
        float CapsuleHalfHeight = Capsule->GetScaledCapsuleHalfHeight();
        // GetActorLocation 通常是胶囊体中心, 这里使用 3/4 位置
        OriginLocation.Z = OriginLocation.Z + CapsuleHalfHeight * 0.5f;
    }
    else
    {
        // 如果没有胶囊体，就默认加个高度
        OriginLocation.Z += 50.0f;
    }

    // 2. 计算扇形目标点
    FVector Forward = OriginActor->GetActorForwardVector();
    float ConeHalfAngleRad = FMath::DegreesToRadians(ConeHalfAngle);

    FVector RandomDir = FMath::VRandCone(Forward, ConeHalfAngleRad);
    RandomDir.Z = 0.0f; // 保持水平方向的随机性，高度由 OriginLocation 决定
    RandomDir.Normalize();

    float RandomDist = FMath::RandRange(MinRadius, MaxRadius);
    FVector DesiredLocation = OriginLocation + (RandomDir * RandomDist);

    // 3. 防穿墙检测 (Visibility Trace)
    // 从起点向目标点发射射线，看看中间有没有墙
    FHitResult Hit;
    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(OriginActor); // 忽略自己

    // 使用 Visibility 或 WorldStatic 通道
    if (World->LineTraceSingleByChannel(Hit, OriginLocation, DesiredLocation, ECC_WorldStatic, QueryParams))
    {
        // 如果撞到了墙，就生成在撞击点前面一点点 (比如回缩 10cm)
        // 这样保证物品紧贴着墙，但不会在墙里面
        OutLocation = Hit.Location + (Hit.ImpactNormal * 10.0f);
    }
    else
    {
        // 没撞到墙，直接用理想点
        OutLocation = DesiredLocation;
    }

    return true;
}

float ULyraSystemStatics::SampleCurveValue(const UCurveFloat* Curve, float X, float DefaultValue)
{
	if (!Curve)
	{
		return DefaultValue;
	}

	return Curve->GetFloatValue(X);
}

int32 ULyraSystemStatics::CalculateDropCount(int32 CurrentStackCount, UCurveFloat* DropCurve)
{
	if (CurrentStackCount <= 0)
	{
		return 0;
	}

	const float Value = SampleCurveValue(DropCurve, static_cast<float>(CurrentStackCount), 1.0f);

	if (Value < 1.0f && Value > 0.0f)
	{
		const int32 Count = FMath::FloorToInt(CurrentStackCount * Value);
		return FMath::Max(1, Count);
	}
	else
	{
		return FMath::Clamp(FMath::RoundToInt(Value), 1, CurrentStackCount);
	}
}

