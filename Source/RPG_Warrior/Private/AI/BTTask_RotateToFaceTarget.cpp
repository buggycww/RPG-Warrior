// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/BTTask_RotateToFaceTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/KismetMathLibrary.h"

UBTTask_RotateToFaceTarget::UBTTask_RotateToFaceTarget()
{
	NodeName = (TEXT("Native Rotate to Face Target Actor"));
	AnglePrecision = 10.f;
	InterpSpeed = 5.f;

	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = false;

	INIT_TASK_NODE_NOTIFY_FLAGS();

	InTargetToFaceKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, InTargetToFaceKey), AActor::StaticClass());
}

void UBTTask_RotateToFaceTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		InTargetToFaceKey.ResolveSelectedKey(*BBAsset);
	}
}

uint16 UBTTask_RotateToFaceTarget::GetInstanceMemorySize() const
{
	return sizeof(FRotateToFaceTargetTaskMemory);
}

FString UBTTask_RotateToFaceTarget::GetStaticDescription() const
{
	const FString KeyDescription = InTargetToFaceKey.SelectedKeyName.ToString();

	return FString::Printf(TEXT("Smoothly rotates to face %s key until angle precision %s is achieved."), *KeyDescription, *FString::SanitizeFloat(AnglePrecision));
}

EBTNodeResult::Type UBTTask_RotateToFaceTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	UObject* ActorObject = OwnerComp.GetBlackboardComponent()->GetValueAsObject(InTargetToFaceKey.SelectedKeyName);
	AActor* TargetActor = Cast<AActor>(ActorObject);

	APawn* OwningPawn = OwnerComp.GetAIOwner()->GetPawn();

	FRotateToFaceTargetTaskMemory* Memory = CastInstanceNodeMemory<FRotateToFaceTargetTaskMemory>(NodeMemory);
	check(Memory);

	Memory->OwningPawn = OwningPawn;
	Memory->TargetActor = TargetActor;

	if (!Memory->IsValid()) 
	{
		return EBTNodeResult::Failed;
	}

	if (HasReachedAnglePrecision(OwningPawn, TargetActor))
	{
		Memory->Reset();
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_RotateToFaceTarget::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FRotateToFaceTargetTaskMemory* Memory = CastInstanceNodeMemory<FRotateToFaceTargetTaskMemory>(NodeMemory);
	
	if (!Memory->IsValid())
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	if (HasReachedAnglePrecision(Memory->OwningPawn.Get(), Memory->TargetActor.Get()))
	{
		Memory->Reset();
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
		return;
	}
	else
	{
		const FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(
			Memory->OwningPawn->GetActorLocation(), 
			Memory->TargetActor->GetActorLocation()
		);

		// for if the AI has to pitch
		/*const FRotator TargetRot = FMath::RInterpTo(Memory->OwningPawn->GetActorRotation(), LookAtRot, DeltaSeconds, InterpSpeed);
		Memory->OwningPawn->SetActorRotation(TargetRot);*/

		// for grounded AI (only yaw)
		// can't use FInterpTo for yaw only bc will face warp issue (RInternTo normalises internally)
		FRotator CurrentRot = Memory->OwningPawn->GetActorRotation();
		FRotator TargetRot = CurrentRot;
		TargetRot.Yaw = LookAtRot.Yaw;
		const FRotator NewRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaSeconds, InterpSpeed);

		Memory->OwningPawn->SetActorRotation(NewRot);
	}
}


// for if the AI has to pitch
/* bool UBTTask_RotateToFaceTarget::HasReachedAnglePrecision(APawn* QueryPawn, AActor* TargetActor) const
{
	const FVector OwningPawnForward = QueryPawn->GetActorForwardVector();
	const FVector DirToTargetActorNormalized = (TargetActor->GetActorLocation() - QueryPawn->GetActorLocation()).GetSafeNormal();

	// pawn and target at same position: nothing to face
	// otherwise if OwningPawn's location == Target Actor's location, then d = 0, d̂ = 0, f · 0 = 0, so arccos(0) = 90°. 
	// So a pawn standing on top of the target reports "not reached, 90° off," which is arbitrary. 
	if (DirToTargetActorNormalized.IsNearlyZero()) return true;

	// cos(θ) = forward · dir  (both unit vectors)
	// "within AnglePrecision degrees" ⟺ θ ≤ θ(AnglePrecision)
	// "within AnglePrecision degrees" ⟺ cos(θ) ≥ cos(AnglePrecision)
	// because cos is strictly decreasing on [0°, 180°].
	const float CosThreshold = FMath::Cos(FMath::DegreesToRadians(AnglePrecision));
	const float Dot = FVector::DotProduct(OwningPawnForward, DirToTargetActorNormalized);

	return Dot >= CosThreshold;
} */

// for grounded AI (only yaw). cheaper and more correct
bool UBTTask_RotateToFaceTarget::HasReachedAnglePrecision(APawn* QueryPawn, AActor* TargetActor) const
{
	const FVector Delta = TargetActor->GetActorLocation() - QueryPawn->GetActorLocation();

	if (Delta.IsNearlyZero())
		return true;

	const float CurrentYaw = QueryPawn->GetActorRotation().Yaw;
	const float DesiredYaw = Delta.Rotation().Yaw;

	// Smallest signed difference in [-180, 180], wrap-safe.
	// FindDeltaAngleDegrees handles the 359° vs 1° wrap problem correctly; the naive abs(a - b) does not.
	const float DeltaYaw = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentYaw, DesiredYaw));

	return DeltaYaw <= AnglePrecision;
}