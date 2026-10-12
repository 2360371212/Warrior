// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCombatComponent.h"

#include "WarriorDebugHelper.h"

void UEnemyCombatComponent::OnHitTargetActor(AActor* HitActor)
{
	if (HitActor)
	{
		Debug::Print(GetOwningPawn()->GetActorNameOrLabel() + TEXT("is hitting") + HitActor->GetActorNameOrLabel());
	}
}

void UEnemyCombatComponent::OnWeaponPulledFromTargetActor(AActor * InteractedActor)
{




}
