// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/WarriorBaseCharacter.h"
#include "WarriorEnemyCharacter.generated.h"

class UEnemyCombatComponent;
class UEnemyUIComponent;
/**
 * 
 */
UCLASS()
class WARRIOR_API AWarriorEnemyCharacter : public AWarriorBaseCharacter
{
	GENERATED_BODY()

public:
	AWarriorEnemyCharacter();

#if WITH_EDITOR
	virtual void OnConstruction(const FTransform& Transform) override;
#endif

	FORCEINLINE UEnemyCombatComponent* GetEnemyCombatComponent() const {
		return EnemyCombatComponent;
	}

	// ~Begin IPawnCombatComponent.
	virtual UPawnCombatComponent* GetPawnCombatComponent() const override;
	// ~End IPawnCombatComponent.

	// ~Begin IPawnPawnUIInterface.
	virtual UPawnUIComponent* GetPawnUIComponent() const override;
	virtual UEnemyUIComponent* GetEnemyUIComponent() const;
	// ~End IPawnPawnUIInterface.

protected:

	// ~Begin APawn Interface.
	virtual void PossessedBy(AController* NewController) override;
	// ~End APawn Interface.


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combato")
	UEnemyCombatComponent* EnemyCombatComponent;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	UEnemyUIComponent* EnemyUIComponent;

private:
	void InitEnemyStartUpData();
};
