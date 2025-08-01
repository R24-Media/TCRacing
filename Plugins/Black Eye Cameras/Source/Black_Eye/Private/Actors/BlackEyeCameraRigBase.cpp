// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.


#include "Actors/BlackEyeCameraRigBase.h"

#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Runtime/Engine/Public/TimerManager.h"

#include "Utility/BlackEyeUtilities.h"

#include "Components/FollowComponent.h"

#include "Interfaces/BlackEyeHasFollow.h"
#include "Interfaces/BlackEyeHasLookAt.h"

// Sets default values
ABlackEyeCameraRigBase::ABlackEyeCameraRigBase()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = ETickingGroup::TG_PostPhysics;

    this->AutoAssignTo = EAutoReceiveInput::Disabled;
    this->bSetPawnAsFollow = true;
    this->bSetPawnAsLookAt = true;
}

#if WITH_EDITOR
void ABlackEyeCameraRigBase::EditorApplyTranslation(const FVector& DeltaTranslation, bool bAltDown, bool bShiftDown, bool bCtrlDown)
{
    if (UFollowComponent* Follow = Cast<UFollowComponent>(this->RootComponent))
    {
        Follow->ProcessActorEditorTranslationDelta(DeltaTranslation);
    }
    Super::EditorApplyTranslation(DeltaTranslation, bAltDown, bShiftDown, bCtrlDown);
}
#endif

void ABlackEyeCameraRigBase::SnapComponentsToTargets()
{
    TArray<UActorComponent*> FollowComponents = GetComponentsByInterface(UBlackEyeHasFollow::StaticClass());
    for (UActorComponent* FollowComp : FollowComponents)
    {
        Cast<IBlackEyeHasFollow>(FollowComp)->SnapToTargets();
    }

    TArray<UActorComponent*> LookAtComponents = GetComponentsByInterface(UBlackEyeHasLookAt::StaticClass());
    for (UActorComponent* LookAtComp : LookAtComponents)
    {
        Cast<IBlackEyeHasLookAt>(LookAtComp)->SnapToTargets();
    }
}

bool ABlackEyeCameraRigBase::ShouldTickIfViewportsOnly() const { return true; }

void ABlackEyeCameraRigBase::BeginPlay()
{
    Super::BeginPlay();

    if (!TryGetCameraComponent())
    {
        UE_LOG(LogTemp, Display, TEXT("No camera was found under camera rig component!"));
    }

    if (UWorld* World = GetWorld())
    {
        if (this->AutoAssignTo == EAutoReceiveInput::Player0)
        {
            World->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this]() {
                APlayerController* PC = Cast<APlayerController>(GetWorld()->GetFirstPlayerController());
                APawn* Pawn = PC->GetPawn();
                PC->PlayerCameraManager->SetViewTarget(this);

                if (this->bSetPawnAsFollow)
                {
                    TArray<UActorComponent*> FollowComponents = GetComponentsByInterface(UBlackEyeHasFollow::StaticClass());
                    for (UActorComponent* FollowComp : FollowComponents)
                    {
                        Cast<IBlackEyeHasFollow>(FollowComp)->SetFollowActorOverride(Pawn);
                    }
                }

                if (this->bSetPawnAsLookAt)
                {
                    TArray<UActorComponent*> LookAtComponents = GetComponentsByInterface(UBlackEyeHasLookAt::StaticClass());
                    for (UActorComponent* LookAtComp : LookAtComponents)
                    {
                        IBlackEyeHasLookAt* LookAtImpl = Cast<IBlackEyeHasLookAt>(LookAtComp);
                        LookAtImpl->SetLookAtActorOverride(Pawn, 0, true);
                    }
                }
                }));
        }
        else if (this->AutoAssignTo != EAutoReceiveInput::Disabled)
        {
            UE_LOG(LogTemp, Display, TEXT("Auto assign of camera rig to any player other than 0 is not supported!"));
        }
    }
}

void ABlackEyeCameraRigBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

#if WITH_EDITOR
    if (GetWorld()->IsEditorWorld() && !this->Camera.IsValid())
    {
        TryGetCameraComponent();
    }
#endif
}

TWeakObjectPtr<UCameraComponent> ABlackEyeCameraRigBase::GetCamera() const
{
    return this->Camera;
}

bool ABlackEyeCameraRigBase::TryGetCameraComponent()
{
    if (!this->Camera.IsValid())
    {
        UCameraComponent* camera = Cast<UCameraComponent>(this->GetComponentByClass(UCameraComponent::StaticClass()));
        this->Camera = camera;
    }

    return this->Camera.IsValid();
}
