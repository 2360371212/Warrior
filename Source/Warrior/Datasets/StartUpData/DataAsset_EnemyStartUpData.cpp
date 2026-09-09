// Fill out your copyright notice in the Description page of Project Settings.


#include "Datasets/StartUpData/DataAsset_EnemyStartUpData.h"
#include "AbilitySystem/WarriorAbilitySystemComponent.h"
#include "AbilitySystem/Abilities/WarriorEnemyGameplayAbility.h"

void UDataAsset_EnemyStartUpData::GiveToAbilitySystemComponent(UWarriorAbilitySystemComponent* InASCToGive, int32 ApplyLevel)
{
	Super::GiveToAbilitySystemComponent(InASCToGive, ApplyLevel);
	if (EnemyCombatAbilities.IsEmpty()) return;

	for (const TSubclassOf<UWarriorEnemyGameplayAbility>& AbilityClass : EnemyCombatAbilities)
	{
		if (!AbilityClass) continue;
		
		FGameplayAbilitySpec AbilitySpec(AbilityClass.Get());
		AbilitySpec.SourceObject = InASCToGive->GetAvatarActor();
		AbilitySpec.Level = ApplyLevel;
		InASCToGive->GiveAbility(AbilitySpec);
	}
}
