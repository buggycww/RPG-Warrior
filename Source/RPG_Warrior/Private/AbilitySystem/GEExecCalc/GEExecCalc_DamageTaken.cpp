// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GEExecCalc/GEExecCalc_DamageTaken.h"
#include "WarriorGameplayTags.h"
#include "AbilitySystem/WarriorAttributeSet.h"

#include "WarriorDebugHelper.h"

struct FWarriorDamageCapture
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower)
	DECLARE_ATTRIBUTE_CAPTUREDEF(DefensePower)
	DECLARE_ATTRIBUTE_CAPTUREDEF(DamageTaken)

	FWarriorDamageCapture()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UWarriorAttributeSet, AttackPower, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UWarriorAttributeSet, DefensePower, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UWarriorAttributeSet, DamageTaken, Target, false);
	}
};

static const FWarriorDamageCapture& GetWarriorDamageCapture()
{
	static FWarriorDamageCapture WarriorDamageCapture;
	return WarriorDamageCapture;
}

UGEExecCalc_DamageTaken::UGEExecCalc_DamageTaken()
{
	RelevantAttributesToCapture.Add(GetWarriorDamageCapture().AttackPowerDef);
	RelevantAttributesToCapture.Add(GetWarriorDamageCapture().DefensePowerDef);
	RelevantAttributesToCapture.Add(GetWarriorDamageCapture().DamageTakenDef);
}

void UGEExecCalc_DamageTaken::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FGameplayEffectSpec& EffectSpec = ExecutionParams.GetOwningSpec();

	FAggregatorEvaluateParameters EvaluateParameters;
	EvaluateParameters.SourceTags = EffectSpec.CapturedSourceTags.GetAggregatedTags();
	EvaluateParameters.TargetTags = EffectSpec.CapturedTargetTags.GetAggregatedTags();

	float SourceAttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetWarriorDamageCapture().AttackPowerDef, EvaluateParameters, SourceAttackPower);
	/*Debug::Print(TEXT("SourceAttackPower"), SourceAttackPower);*/

	float BaseDamage;
	int32 CurrentLightAttackComboCount = 0;
	int32 CurrentHeavyAttackComboCount = 0;
	for (const TPair<FGameplayTag, float>& TMagnitude : EffectSpec.SetByCallerTagMagnitudes)
	{
		if (TMagnitude.Key.MatchesTagExact(WarriorGameplayTags::Shared_SetByCaller_BaseDamage))
		{
			BaseDamage = TMagnitude.Value;
			/*Debug::Print(TEXT("BaseDamage"), BaseDamage);*/
		}

		if (TMagnitude.Key.MatchesTagExact(WarriorGameplayTags::Player_SetByCaller_Attack_Light))
		{
			CurrentLightAttackComboCount = TMagnitude.Value;
			/*Debug::Print(TEXT("CurrentLightAttackComboCount"), CurrentLightAttackComboCount);*/
		}

		if (TMagnitude.Key.MatchesTagExact(WarriorGameplayTags::Player_SetByCaller_Attack_Heavy))
		{
			CurrentHeavyAttackComboCount = TMagnitude.Value;
			/*Debug::Print(TEXT("CurrentHeavyAttackComboCount"), CurrentHeavyAttackComboCount);*/
		}
	}

	float TargetDefensePower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetWarriorDamageCapture().DefensePowerDef, EvaluateParameters, TargetDefensePower);
	/*Debug::Print(TEXT("TargetDefensePower"), TargetDefensePower);*/

	if (CurrentLightAttackComboCount > 0)
	{
		const float DamageIncreasePercentLight = (CurrentLightAttackComboCount - 1) * 0.05f + 1.f;
		BaseDamage *= DamageIncreasePercentLight;
		/*Debug::Print(TEXT("ScaledBaseDamageLight"), BaseDamage);*/
	}

	if (CurrentHeavyAttackComboCount > 0)
	{
		const float DamageIncreasePercentHeavy = CurrentHeavyAttackComboCount * 0.15f + 1.f;
		BaseDamage *= DamageIncreasePercentHeavy;
		/*Debug::Print(TEXT("ScaledBaseDamageHeavy"), BaseDamage);*/
	}

	const float FinalDamageDone = BaseDamage * (SourceAttackPower / TargetDefensePower);
	/*Debug::Print(TEXT("FinalDamageDone"), FinalDamageDone);*/

	if (FinalDamageDone > 0.f)
	{
		FGameplayModifierEvaluatedData DamageModifierData(GetWarriorDamageCapture().AttackPowerDef.AttributeToCapture, EGameplayModOp::Additive, FinalDamageDone);
		OutExecutionOutput.AddOutputModifier(
			FGameplayModifierEvaluatedData(
				GetWarriorDamageCapture().DamageTakenProperty,
				EGameplayModOp::Override,
				FinalDamageDone
			)
		);
	}
}

