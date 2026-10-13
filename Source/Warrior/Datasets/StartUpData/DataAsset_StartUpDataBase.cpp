// Fill out your copyright notice in the Description page of Project Settings.


#include "DataAsset_StartUpDataBase.h"
#include "AbilitySystem/Abilities/WarriorGameplayAbility.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"

void UDataAsset_StartUpDataBase::GiveToAbilitySystemComponent(UWarriorAbilitySystemComponent* InASCToGive, int32 ApplyLevel)
{
	check(InASCToGive);

	// 授予初始就激活的能力
	GrantAbility(ActivateOnGivenAbilityes, InASCToGive, ApplyLevel);

	// 授予战斗反馈能力
	GrantAbility(ReactiveAbilities, InASCToGive, ApplyLevel);

	// 授予GameplayEffects
	if (!StartUpGameplayEffects.IsEmpty())
	{
		for (const TSubclassOf<UGameplayEffect>& EffectClass : StartUpGameplayEffects)
		{
			if (!EffectClass) continue;
			UGameplayEffect* EffectCDO =  EffectClass->GetDefaultObject<UGameplayEffect>();
			InASCToGive->ApplyGameplayEffectToSelf(EffectCDO, ApplyLevel, InASCToGive->MakeEffectContext());
		}
	}
}

void UDataAsset_StartUpDataBase::GrantAbility(TArray<TSubclassOf<UWarriorGameplayAbility>>& InAbilityesToGive, UWarriorAbilitySystemComponent* InASCToGive, int32 ApplyLevel)
{
	if (InAbilityesToGive.IsEmpty())
	{
		return;
	}
	for (const TSubclassOf<UWarriorGameplayAbility>& Ability : InAbilityesToGive)
	{
		if (!Ability)
		{
			continue;
		}

		// 对Gameplay Ability进行封装
		FGameplayAbilitySpec AbilitySpec(Ability);
		AbilitySpec.SourceObject = InASCToGive->GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;

		InASCToGive->GiveAbility(AbilitySpec);
	}
}
