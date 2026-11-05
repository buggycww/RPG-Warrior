// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/HeroCombatComponent.h"
#include "Items/Weapons/WarriorHeroWeapon.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "WarriorGameplayTags.h"

#include "WarriorDebugHelper.h"

AWarriorHeroWeapon* UHeroCombatComponent::GetHeroCarriedWeaponByTag(FGameplayTag InWeaponTag) const
{
    return Cast<AWarriorHeroWeapon>(GetCharacterCarriedWeaponByTag(InWeaponTag));
}

void UHeroCombatComponent::OnBeginHitTarget(AActor* InTargetActor)
{
	if (OverlappedActors.Contains(InTargetActor)) return;

	OverlappedActors.AddUnique(InTargetActor);

	FGameplayEventData Data;
	Data.Instigator = GetOwningPawn();
	Data.Target = InTargetActor;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
		GetOwningPawn(), 
		WarriorGameplayTags::Shared_Event_MeleeHit,
		Data
	);
}

void UHeroCombatComponent::OnEndHitTarget(AActor* InTargetActor)
{
}
