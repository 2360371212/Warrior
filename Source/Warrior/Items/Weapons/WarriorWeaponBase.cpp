// Fill out your copyright notice in the Description page of Project Settings.


#include "WarriorWeaponBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"

#include "WarriorDebugHelper.h"

// Sets default values
AWarriorWeaponBase::AWarriorWeaponBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	SetRootComponent(WeaponMesh);
	WeaponCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponCollisionBox"));
	WeaponCollisionBox->SetupAttachment(WeaponMesh);
	
	WeaponCollisionBox->SetBoxExtent(FVector(20.f));
	WeaponCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	WeaponCollisionBox->OnComponentBeginOverlap.AddUniqueDynamic(this, &AWarriorWeaponBase::OnCollisionBoxBeginOverlap);
	WeaponCollisionBox->OnComponentEndOverlap.AddUniqueDynamic(this, &AWarriorWeaponBase::OnCollisionBoxEndOverlap);
}

void AWarriorWeaponBase::OnCollisionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APawn* WeaponOwingPawn = GetInstigator<APawn>();
	checkf(WeaponOwingPawn, TEXT("Forget to assign an instiagtor as the owning pawn for the weapon: %s"), *GetName());
	if (APawn* HitPawn = Cast<APawn>(OtherActor))
	{
		if (WeaponOwingPawn != HitPawn)
		{
			OnWeaponHitTarget.ExecuteIfBound(OtherActor);
		}
		//TODO:Implement hit check for enemy characters
	}

}

void AWarriorWeaponBase::OnCollisionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{		
	APawn* WeaponOwingPawn = GetInstigator<APawn>();
	checkf(WeaponOwingPawn, TEXT("Forget to assign an instiagtor as the owning pawn for the weapon: %s"), *GetName());
	if (APawn* HitPawn = Cast<APawn>(OtherActor))
	{
		if (WeaponOwingPawn != HitPawn)
		{
			OnWeaponPulledFromTarget.ExecuteIfBound(OtherActor);
		}
		//TODO:Implement hit check for enemy characters
	}
			
}


