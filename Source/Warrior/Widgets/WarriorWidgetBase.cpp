// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/WarriorWidgetBase.h"
#include "Interfaces/PawnUIInterface.h"
#include "Components/UI/PawnUIComponent.h"

void UWarriorWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();


	if (IPawnUIInterface* PawnUIInterface = Cast<IPawnUIInterface>(GetOwningPlayerPawn()))
	{
		if (UHeroUIComponent* HeroUIComponent = PawnUIInterface->GetHeroUIComponent()) {
			BP_OnOwningHeroUIComponentInitialized(HeroUIComponent);
		}
	}


}

void UWarriorWidgetBase::InitEnemyCreateWidget(AActor* OwningEnemyActor)
{
	if (IPawnUIInterface* PawnUIInterface = Cast<IPawnUIInterface>(OwningEnemyActor)) {
		UEnemyUIComponent* EnemyUIConponent = PawnUIInterface->GetEnemyUIComponent();

		checkf(EnemyUIConponent, TEXT("Failed to extrac an EnemyUIComponent from %s"), *OwningEnemyActor->GetActorNameOrLabel());

		BP_OnOwningEnemyUIComponentInitialized(EnemyUIConponent);
	}
}
