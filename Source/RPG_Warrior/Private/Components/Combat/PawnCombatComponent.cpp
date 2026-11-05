// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/PawnCombatComponent.h"
#include "Items/Weapons/WarriorWeaponBase.h"
#include "Components/BoxComponent.h"

#include "WarriorDebugHelper.h"

void UPawnCombatComponent::RegisterSpawnedWeapon(const FGameplayTag& InWeaponTagToRegister, AWarriorWeaponBase* InWeaponToRegister, bool bRegisterAsEquippedWeapon)
{
	checkf(!CharacterCarriedWeaponMap.Contains(InWeaponTagToRegister), TEXT("A tag names %s has already been added as carried weapon."), *InWeaponTagToRegister.GetTagName().ToString());
	check(InWeaponToRegister);

	CharacterCarriedWeaponMap.Emplace(InWeaponTagToRegister, InWeaponToRegister);

	InWeaponToRegister->OnWeaponBeginHitTarget.BindUObject(this, &ThisClass::OnBeginHitTarget);
	InWeaponToRegister->OnWeaponEndHitTarget.BindUObject(this, &ThisClass::OnEndHitTarget);

	if (bRegisterAsEquippedWeapon)
	{
		CurrentEquippedWeaponTag = InWeaponTagToRegister;
	}
}

AWarriorWeaponBase* UPawnCombatComponent::GetCharacterCarriedWeaponByTag(const FGameplayTag& InWeaponTagToGet) const
{
	if (AWarriorWeaponBase* const* FoundWeapon = CharacterCarriedWeaponMap.Find(InWeaponTagToGet))
	{
		return *FoundWeapon;
	}

	return nullptr;
}

AWarriorWeaponBase* UPawnCombatComponent::GetCharacterCurrentEquippedWeapon() const
{
	if (!CurrentEquippedWeaponTag.IsValid())
	{
		return nullptr;
	}

	return *CharacterCarriedWeaponMap.Find(CurrentEquippedWeaponTag);
}

void UPawnCombatComponent::ToggleWeaponCollision(bool bShouldEnable, EToggleDamageType ToggleDamageType)
{
	switch (ToggleDamageType)
	{
		case EToggleDamageType::CurrentEquippedWeapon:
		{
			AWarriorWeaponBase* WeaponToTogle = GetCharacterCurrentEquippedWeapon();
			check(WeaponToTogle);

			if (bShouldEnable)
			{
				WeaponToTogle->GetWeaponCollisionBox()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			}
			else
			{
				WeaponToTogle->GetWeaponCollisionBox()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				OverlappedActors.Empty();
			}

			break;
		}
	}

	// TODO: Implement LeftHand and RightHand toggle damage type.
}

void UPawnCombatComponent::OnBeginHitTarget(AActor* InTargetActor)
{
}

void UPawnCombatComponent::OnEndHitTarget(AActor* InTargetActor)
{
}
