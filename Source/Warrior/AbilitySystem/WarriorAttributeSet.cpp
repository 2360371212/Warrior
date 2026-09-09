// Fill out your copyright notice in the Description page of Project Settings.


#include "WarriorAttributeSet.h"

UWarriorAttributeSet::UWarriorAttributeSet()
{
	InitCurrentHealth(1.f);
	InitMaxHealth(1.f);
	InitCurrentRage(1.0f);
	InitMaxRage(1.0f);
	InitAttackPower(1.f);
	InitDefensePower(1.f);
}
