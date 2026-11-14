// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/EnemyCombatComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "WarriorGameplayTags.h"

#include "WarriorDebugHelper.h"

void UEnemyCombatComponent::OnBeginHitTarget(AActor* InTargetActor)
{
	if (OverlappedActors.Contains(InTargetActor))
	{
		return;
	}

	OverlappedActors.AddUnique(InTargetActor);

	//TODO::implement block check
	bool isValidBlock = false;
	
	bool isPlayerBlocking = false;
	bool isMyAttackUnblockable = false;

	if (isPlayerBlocking && !isMyAttackUnblockable)
	{
		//TODO::check if block is valid
	}

	FGameplayEventData EventData;
	EventData.Instigator = GetOwningPawn();
	EventData.Target = InTargetActor;

	if (isValidBlock)
	{
		//TODO::handle successful block
	}
	else
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			GetOwningPawn(),
			WarriorGameplayTags::Shared_Event_MeleeHit,
			EventData
		);
	}
}
