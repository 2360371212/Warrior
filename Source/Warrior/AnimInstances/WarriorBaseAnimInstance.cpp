// Fill out your copyright notice in the Description page of Project Settings.


#include "WarriorBaseAnimInstance.h"
#include "WarriorFunctionLibrary.h"

bool UWarriorBaseAnimInstance::DoesOwnerHaveTag(FGameplayTag TagToCheck) const
{
	// 尝试获取当前这个 AnimInstance 所属 SkeletalMesh 对应的 Pawn
	if (APawn* OwningPawn = TryGetPawnOwner()) {
		return UWarriorFunctionLibrary::NativeDoesActorHasTag(OwningPawn, TagToCheck);
	}
	return false;
}
