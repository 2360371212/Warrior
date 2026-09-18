// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/WarriorBaseCharacter.h"
#include "WarriorEnemyCharacter.generated.h"

class UEnemyCombatComponent;
/**
 * 
 */
UCLASS()
class WARRIOR_API AWarriorEnemyCharacter : public AWarriorBaseCharacter
{
	GENERATED_BODY()

public:
	AWarriorEnemyCharacter();

	FORCEINLINE UEnemyCombatComponent* GetEnemyCombatComponent() const {
		return EnemyCombatComponent;
	}

	// ~Begin IPawnCombatComponent.
	virtual UPawnCombatComponent* GetPawnCombatComponent() const override;
	// ~End IPawnCombatComponent.

protected:

	// ~Begin APawn Interface.
	virtual void PossessedBy(AController* NewController) override;
	// ~End APawn Interface.


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combato")
	UEnemyCombatComponent* EnemyCombatComponent;

private:
	void InitEnemyStartUpData();
};
