// Copyright Epic Games, Inc. All Rights Reserved.

#include "LyraGameplayTags.h"

#include "Engine/EngineTypes.h"
#include "GameplayTagsManager.h"
#include "LyraLogChannels.h"

namespace LyraGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_IsDead, "Ability.ActivateFail.IsDead", "Ability failed to activate because its owner is dead.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cooldown, "Ability.ActivateFail.Cooldown", "Ability failed to activate because it is on cool down.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Cost, "Ability.ActivateFail.Cost", "Ability failed to activate because it did not pass the cost checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsBlocked, "Ability.ActivateFail.TagsBlocked", "Ability failed to activate because tags are blocking it.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_TagsMissing, "Ability.ActivateFail.TagsMissing", "Ability failed to activate because tags are missing.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_Networking, "Ability.ActivateFail.Networking", "Ability failed to activate because it did not pass the network checks.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_ActivateFail_ActivationGroup, "Ability.ActivateFail.ActivationGroup", "Ability failed to activate because of its activation group.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Behavior_SurvivesDeath, "Ability.Behavior.SurvivesDeath", "An ability with this type tag should not be canceled due to death.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Move, "InputTag.Move", "Move input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Mouse, "InputTag.Look.Mouse", "Look (mouse) input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Look_Stick, "InputTag.Look.Stick", "Look (stick) input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_Crouch, "InputTag.Crouch", "Crouch input.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputTag_AutoRun, "InputTag.AutoRun", "Auto-run input.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_Spawned, "InitState.Spawned", "1: Actor/component has initially spawned and can be extended");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataAvailable, "InitState.DataAvailable", "2: All required data has been loaded/replicated and is ready for initialization");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_DataInitialized, "InitState.DataInitialized", "3: The available data has been initialized for this actor/component, but it is not ready for full gameplay");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(InitState_GameplayReady, "InitState.GameplayReady", "4: The actor/component is fully ready for active gameplay");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Death, "GameplayEvent.Death", "Event that fires on death. This event only fires on the server.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Reset, "GameplayEvent.Reset", "Event that fires once a player reset is executed.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_RequestReset, "GameplayEvent.RequestReset", "Event to request a player's pawn to be instantly replaced with a new one at a valid spawn location.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Damage, "SetByCaller.Damage", "SetByCaller tag used by damage gameplay effects.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(SetByCaller_Heal, "SetByCaller.Heal", "SetByCaller tag used by healing gameplay effects.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cheat_GodMode, "Cheat.GodMode", "GodMode cheat is active on the owner.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Cheat_UnlimitedHealth, "Cheat.UnlimitedHealth", "UnlimitedHealth cheat is active on the owner.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Crouching, "Status.Crouching", "Target is crouching.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_AutoRunning, "Status.AutoRunning", "Target is auto-running.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death, "Status.Death", "Target has the death status.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dying, "Status.Death.Dying", "Target has begun the death process.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Status_Death_Dead, "Status.Death.Dead", "Target has finished the death process.");
						  
	// These are mapped to the movement modes inside GetMovementModeTagMap()
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Walking, "Movement.Mode.Walking", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_NavWalking, "Movement.Mode.NavWalking", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Falling, "Movement.Mode.Falling", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Swimming, "Movement.Mode.Swimming", "Default Character movement tag");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Flying, "Movement.Mode.Flying", "Default Character movement tag");

	// When extending Lyra, you can create your own movement modes but you need to update GetCustomMovementModeTagMap()
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Movement_Mode_Custom, "Movement.Mode.Custom", "This is invalid and should be replaced with custom tags.  See LyraGameplayTags::CustomMovementModeTagMap.");




	// GameplayTag
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gameplay_Message_Nameplate_Add, "Gameplay.Message.Nameplate.Add", "Indicates a request to add/display a nameplate for an entity.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gameplay_Message_Nameplate_Remove, "Gameplay.Message.Nameplate.Remove", "Indicates a request to remove/hide a nameplate for an entity.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gameplay_Message_Nameplate_Discover, "Gameplay.Message.Nameplate.Discover", "Indicates a new entity has been discovered (e.g., entered vision), typically followed by an Add request.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gameplay_Message_Marker_Add, "Gameplay.Message.Marker.Add", "Indicates a request to add/display a world marker.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gameplay_Message_Marker_Remove, "Gameplay.Message.Marker.Remove", "Indicates a request to remove/hide a world marker");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gameplay_Message_Marker_Discover, "Gameplay.Message.Marker.Discover", "Indicates a new world marker has been discovered, typically followed by an Add request.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gameplay_Message_Callout_Display, "Gameplay.Message.Callout.Display", "Indicates a request to display a UI callout.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gameplay_Message_Session_MemberEvent, "Gameplay.Message.Session.MemberEvent", "Toast message for when a member joins or leaves the active game session.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gameplay_Message_Lobby_MemberEvent, "Gameplay.Message.Lobby.MemberEvent", "Toast message for when a member joins or leaves the current lobby.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gameplay_Message_Lobby_Countdown, "Gameplay.Message.Lobby.Countdown", "Toast message for displaying a countdown timer in the lobby.");

	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gameplay_Message_Inventory_StackChanged, "Gameplay.Message.Inventory.StackChanged", "Indicates that an inventory stack has changed.");


	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Gameplay_Message_UI_FocusNavi, "Gameplay.Message.UI.FocusNavi", "Used to indicate which UI element currently has navigation focus.");



	// Interaction Ability Tags
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Interaction_Placeholder, "Ability.Interaction.Placeholder", "Placeholder tag for interaction abilities.");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Interaction_Duration_Message, "Ability.Interaction.Duration.Message", "Message sent when an interaction with a duration starts or ends.");
	// 拾取道具(主动)
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Interaction_Pickup_Active, "Ability.Interaction.Pickup.Active", "Ability tag for active pickups (e.g., player-initiated).");
	// 拾取道具(自动)
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(Ability_Interaction_Pickup_Auto, "Ability.Interaction.Pickup.Auto", "Ability tag for automatic pickups (e.g., proximity-based).");

	// 触发丢弃道具技能 丢弃部分 / 全部
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Inventory_DropItem, "GameplayEvent.Inventory.DropItem", "Event to trigger dropping an item from the inventory.");
	// 触发交换道具位置技能
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Inventory_SwapItem, "GameplayEvent.Inventory.SwapItem", "Event to trigger swapping item positions within the inventory.");
	// 触发道具堆叠技能
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(GameplayEvent_Inventory_StackItem, "GameplayEvent.Inventory.StackItem", "Event to trigger stacking items within the inventory.");



	// Unreal Movement Modes
	const TMap<uint8, FGameplayTag> MovementModeTagMap =
	{
		{ MOVE_Walking, Movement_Mode_Walking },
		{ MOVE_NavWalking, Movement_Mode_NavWalking },
		{ MOVE_Falling, Movement_Mode_Falling },
		{ MOVE_Swimming, Movement_Mode_Swimming },
		{ MOVE_Flying, Movement_Mode_Flying },
		{ MOVE_Custom, Movement_Mode_Custom }
	};

	// Custom Movement Modes
	const TMap<uint8, FGameplayTag> CustomMovementModeTagMap =
	{
		// Fill these in with your custom modes
	};

	FGameplayTag FindTagByString(const FString& TagString, bool bMatchPartialString)
	{
		const UGameplayTagsManager& Manager = UGameplayTagsManager::Get();
		FGameplayTag Tag = Manager.RequestGameplayTag(FName(*TagString), false);

		if (!Tag.IsValid() && bMatchPartialString)
		{
			FGameplayTagContainer AllTags;
			Manager.RequestAllGameplayTags(AllTags, true);

			for (const FGameplayTag& TestTag : AllTags)
			{
				if (TestTag.ToString().Contains(TagString))
				{
					UE_LOG(LogLyra, Display, TEXT("Could not find exact match for tag [%s] but found partial match on tag [%s]."), *TagString, *TestTag.ToString());
					Tag = TestTag;
					break;
				}
			}
		}

		return Tag;
	}
}

