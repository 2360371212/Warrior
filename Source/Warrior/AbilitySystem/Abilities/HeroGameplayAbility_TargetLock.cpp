// Fill out your copyright notice in the Description page of Project Settings.


#include "HeroGameplayAbility_TargetLock.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Characters/WarriorHeroCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/WarriorWidgetBase.h"
#include "Controllers/WarriorHeroController.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/SizeBox.h"
#include "WarriorFunctionLibrary.h"
#include "WarriorGameplayTags.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"


#include "WarriorDebugHelper.h"


void UHeroGameplayAbility_TargetLock::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	// 尝试锁定目标
	TryLockOnTarget();
	// TryLockOnTarget cancels the ability when the search is empty. Do not
	// continue initializing target-lock state after that cancellation: doing so
	// leaves the movement rotation mode/mapping context active with no target,
	// which makes the camera appear stuck after all enemies are dead.
	if (!IsValid(CurrentLockedActor))
	{
		return;
	}

	InitTargetLockMovement();
	InitTargetLockMappingContext();
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
}

void UHeroGameplayAbility_TargetLock::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo * ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	ResetTargetLockMovement();
	ResetTargetLockMappingContext();
	ClearUp();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UHeroGameplayAbility_TargetLock::OnTargetLockTick(float DeltaTime)
{
	AWarriorHeroCharacter* HeroCharacter = GetHeroCharacterFromActorInfo();
	AWarriorHeroController* HeroController = GetHeroControllerFromActorInfo();

	if (!IsValid(HeroCharacter) || !IsValid(HeroController) || !IsValid(CurrentLockedActor) ||
		UWarriorFunctionLibrary::NativeDoesActorHasTag(CurrentLockedActor, WarriorGameplayTags::Shared_Ability_Death) ||
		UWarriorFunctionLibrary::NativeDoesActorHasTag(HeroCharacter, WarriorGameplayTags::Shared_Ability_Death))
	{
		CancelTargetLockAbility();
		return;
	}

	SetTargetLockWidgetPosition();
	// SetTargetLockWidgetPosition can cancel the ability and clear CurrentLockedActor.
	if (!IsValid(CurrentLockedActor))
	{
		return;
	}

	const bool bShouldOverrideRotation =
		!UWarriorFunctionLibrary::NativeDoesActorHasTag(HeroCharacter, WarriorGameplayTags::Player_Status_Rolling) &&
		!UWarriorFunctionLibrary::NativeDoesActorHasTag(HeroCharacter, WarriorGameplayTags::Player_Status_Blocking);

	// 调整锁定后的相机与控制器旋转
				
	if (bShouldOverrideRotation)
	{
		FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(
			HeroCharacter->GetActorLocation(),
			CurrentLockedActor->GetActorLocation()
		);

		LookAtRot -= FRotator(TargetLockCameraOffsetDistance, 0.f, 0.f);

		const FRotator CurrentControlRot = HeroController->GetControlRotation();
		const FRotator TargetRot = FMath::RInterpTo(CurrentControlRot, LookAtRot, DeltaTime, TargetLockRotationInterSpeed);
		HeroController->SetControlRotation(FRotator(TargetRot.Pitch, TargetRot.Yaw, 0.f));
		HeroCharacter->SetActorRotation(FRotator(0.f, TargetRot.Yaw, 0.f));
	}
}

void UHeroGameplayAbility_TargetLock::SwitchTarget(const FGameplayTag& InSwitchDirectionTag)
{
	GetAvailiableActorsToLock();
	TArray<AActor*> ActorOnLeft;
	TArray<AActor*> ActorOnRight;

	AActor* NewTargetToLock = nullptr;
	GetAvailiableActorsAroundTarget(ActorOnLeft, ActorOnRight);

	if (InSwitchDirectionTag == WarriorGameplayTags::Player_Event_SwitchTarget_Left)
	{
		NewTargetToLock = GetNearestTargetFromAvailableActors(ActorOnLeft);
	}
	else {
		NewTargetToLock = GetNearestTargetFromAvailableActors(ActorOnRight);
	}
	if (NewTargetToLock)
	{
		CurrentLockedActor = NewTargetToLock;
	}
}



void UHeroGameplayAbility_TargetLock::TryLockOnTarget()
{
	CurrentLockedActor = nullptr;
	GetAvailiableActorsToLock();
	// 如果当前没有可锁定的目标，则取消英雄能力
	if (AvailableActorsToLock.IsEmpty())
	{
		CancelTargetLockAbility();
		return;
	}
	CurrentLockedActor = GetNearestTargetFromAvailableActors(AvailableActorsToLock);

	if (CurrentLockedActor)
	{
		DrawTargetLockWidget();
		SetTargetLockWidgetPosition();
	}
	// 如果锁定的对象无效则取消当前技能
	else
	{
		CancelTargetLockAbility();
	}
}

void UHeroGameplayAbility_TargetLock::GetAvailiableActorsToLock()
{
	// 在调用之前清空数组
	AvailableActorsToLock.Empty();

	TArray<FHitResult> BoxTraceHits;

	UKismetSystemLibrary::BoxTraceMultiForObjects(
		GetHeroCharacterFromActorInfo(),
		GetHeroCharacterFromActorInfo()->GetActorLocation(),
		GetHeroCharacterFromActorInfo()->GetActorLocation() + GetHeroCharacterFromActorInfo()->GetActorForwardVector() * BoxTraceDistance,
		TraceBoxSize / 2.f,
		GetHeroCharacterFromActorInfo()->GetActorForwardVector().ToOrientationRotator(),
		BoxTraceChannel,
		false,
		TArray<AActor*>(),
		bShowPersistentDebugShape ? EDrawDebugTrace::Persistent : EDrawDebugTrace::None,
		BoxTraceHits,
		true
	);

	for (const FHitResult& TraceHit : BoxTraceHits)
	{
		AActor* HitActor = TraceHit.GetActor();
		if (!IsValid(HitActor) || HitActor == GetHeroCharacterFromActorInfo())
		{
			continue;
		}

		UAbilitySystemComponent* HitASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		// 获取所有可用的目标
		if (IsValid(HitASC) && !HitASC->HasMatchingGameplayTag(WarriorGameplayTags::Shared_Ability_Death))
		{
			AvailableActorsToLock.AddUnique(HitActor);
		}
	}
}

AActor* UHeroGameplayAbility_TargetLock::GetNearestTargetFromAvailableActors(const TArray<AActor*>& InAvailableActors)
{
	float closestDistance = 0.f;
	return UGameplayStatics::FindNearestActor(GetHeroCharacterFromActorInfo()->GetActorLocation(), InAvailableActors, closestDistance);
}

void UHeroGameplayAbility_TargetLock::GetAvailiableActorsAroundTarget(TArray<AActor*>& OutActorsOnLeft, TArray<AActor*>& OutActorsOnRight)
{
	if (!CurrentLockedActor || AvailableActorsToLock.IsEmpty())
	{
		CancelTargetLockAbility();
		return;
	}

	const FVector PlayerLocation = GetHeroCharacterFromActorInfo()->GetActorLocation();
	const FVector PlayerToCurrentNormalized = (CurrentLockedActor->GetActorLocation() - PlayerLocation).GetSafeNormal();

	for (AActor* AvailableActor : AvailableActorsToLock) {
		if (!IsValid(AvailableActor) || AvailableActor == CurrentLockedActor) continue;

		const FVector PlayerToAvailableNormalized = (AvailableActor->GetActorLocation() - PlayerLocation).GetSafeNormal();
		const FVector CrossResult = FVector::CrossProduct(PlayerToCurrentNormalized, PlayerToAvailableNormalized);
		if (CrossResult.Z>0.f)
		{
			OutActorsOnRight.AddUnique(AvailableActor);
		}
		else {
			OutActorsOnLeft.AddUnique(AvailableActor);
		}
	}
}

// 绘制锁定控件
void UHeroGameplayAbility_TargetLock::DrawTargetLockWidget()
{
	if (!DrawnTargetLockWidget)
	{
		checkf(TargetLockWidgetClass, TEXT("Forget to assign a valid widget class to in Blueprint"));
		DrawnTargetLockWidget = CreateWidget<UWarriorWidgetBase>(GetHeroControllerFromActorInfo(), TargetLockWidgetClass);
		check(DrawnTargetLockWidget);
		DrawnTargetLockWidget->AddToViewport();
	}
	
}

// 计算图标尺寸，使其剧中
void UHeroGameplayAbility_TargetLock::SetTargetLockWidgetPosition()
{
	if (!DrawnTargetLockWidget || !CurrentLockedActor)
	{
		CancelTargetLockAbility();
		return;
	}

	FVector2D ScreenPosition;
	UWidgetLayoutLibrary::ProjectWorldLocationToWidgetPosition(
		GetHeroControllerFromActorInfo(),
		CurrentLockedActor->GetActorLocation(),
		ScreenPosition,
		true
	);

	if (TargetLockWidgetSize == FVector2D::ZeroVector)
	{
		DrawnTargetLockWidget->WidgetTree->ForEachWidget([this](UWidget* FoundWidget) {
			if (USizeBox* FoundSizeBox = Cast<USizeBox>(FoundWidget))
			{
				TargetLockWidgetSize.X = FoundSizeBox->GetWidthOverride();
				TargetLockWidgetSize.Y = FoundSizeBox->GetHeightOverride();
			}
		});

	}

	ScreenPosition -= (TargetLockWidgetSize / 2.f);
	DrawnTargetLockWidget->SetPositionInViewport(ScreenPosition, false);

}

void UHeroGameplayAbility_TargetLock::InitTargetLockMovement()
{
	AWarriorHeroCharacter* HeroCharacter = GetHeroCharacterFromActorInfo();
	if (!IsValid(HeroCharacter))
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = HeroCharacter->GetCharacterMovement();
	if (!IsValid(MovementComponent))
	{
		return;
	}

	CachedDefaultMaxWalkSpeed = MovementComponent->MaxWalkSpeed;
	bCachedOrientRotationToMovement = MovementComponent->bOrientRotationToMovement;
	bCachedUseControllerDesiredRotation = MovementComponent->bUseControllerDesiredRotation;
	bTargetLockMovementInitialized = true;

	// Target lock drives the actor rotation explicitly in OnTargetLockTick. Letting
	// CharacterMovementComponent orient from movement at the same time causes the
	// actor yaw and locomotion direction to fight each other, which can leave the
	// lower-body blend moving in place.
	MovementComponent->bOrientRotationToMovement = false;
	MovementComponent->bUseControllerDesiredRotation = false;

	MovementComponent->MaxWalkSpeed = TargetLockMaxWalkSpeed;
}

// 取消当前能力
void UHeroGameplayAbility_TargetLock::CancelTargetLockAbility()
{
	CancelAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo(), true);
}

void UHeroGameplayAbility_TargetLock::ClearUp()
{
	AvailableActorsToLock.Empty();
	CurrentLockedActor = nullptr;
	if (DrawnTargetLockWidget)
	{
		DrawnTargetLockWidget->RemoveFromParent();
	}
	DrawnTargetLockWidget = nullptr;
	TargetLockWidgetSize = FVector2D::ZeroVector;

	CachedDefaultMaxWalkSpeed = 0.f;
	bTargetLockMovementInitialized = false;
}

void UHeroGameplayAbility_TargetLock::ResetTargetLockMovement()
{
	AWarriorHeroCharacter* HeroCharacter = GetHeroCharacterFromActorInfo();
	if (!IsValid(HeroCharacter) || !bTargetLockMovementInitialized)
	{
		return;
	}

	UCharacterMovementComponent* MovementComponent = HeroCharacter->GetCharacterMovement();
	if (!IsValid(MovementComponent))
	{
		return;
	}

	if (CachedDefaultMaxWalkSpeed > 0.f)
	{
		MovementComponent->MaxWalkSpeed = CachedDefaultMaxWalkSpeed;
	}
	MovementComponent->bOrientRotationToMovement = bCachedOrientRotationToMovement;
	MovementComponent->bUseControllerDesiredRotation = bCachedUseControllerDesiredRotation;
}

void UHeroGameplayAbility_TargetLock::InitTargetLockMappingContext()
{
	if (!TargetLockMappingContext)
	{
		return;
	}

	const ULocalPlayer* LocalPlayer = GetHeroControllerFromActorInfo()->GetLocalPlayer();
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (Subsystem)
	{
		Subsystem->AddMappingContext(TargetLockMappingContext, 3);
	}
}

// 移除映射上下文
void UHeroGameplayAbility_TargetLock::ResetTargetLockMappingContext()
{
	if (!GetHeroControllerFromActorInfo())
	{
		return;
	}
	const ULocalPlayer* LocalPlayer = GetHeroControllerFromActorInfo()->GetLocalPlayer();

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer);
	if (Subsystem && TargetLockMappingContext)
	{
		Subsystem->RemoveMappingContext(TargetLockMappingContext);
	}
}


