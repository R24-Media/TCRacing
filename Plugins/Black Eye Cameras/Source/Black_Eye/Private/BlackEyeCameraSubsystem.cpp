// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#include "BlackEyeCameraSubsystem.h"

#include "Actors/BlackEyeCameraRigBase.h"

#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "SceneView.h"
#include "TimerManager.h"

#include "Engine/Canvas.h"

#include "Runtime/Engine/Classes/Engine/GameViewportClient.h"
#include "Runtime/Engine/Classes/Engine/LocalPlayer.h"

#include "CoreMinimal.h"

#include "GameFramework/HUD.h"
#include "GameFramework/PlayerController.h"
#include "UnrealClient.h"

#include "BlackEyeCVars.h"
#include "Utility/BlackEyeMath.h"
#include "Utility/BlackEyeUtilities.h"
#include "Components/FollowComponent.h"
#include "Components/LookAtComponent.h"

#ifndef ENGINE_MINOR_VERSION
#include "Runtime/Launch/Resources/Version.h"
#endif

#if WITH_EDITOR
#include "Editor/UnrealEd/Public/LevelEditorViewport.h"
//#include "UnrealEd.h"
#endif

static FColor RegalLinear = FColor(38, 67, 118);
static FColor RegalGamma = FColor(108, 140, 181);

static FColor CoralRedLinear = FColor(218, 76, 77);
static FColor CoralRedGamma = FColor(239, 149, 149);

static FColor SugarLinear = FColor(241, 221, 222);
static FColor SugarGamma = FColor(249, 239, 240);

static FColor LookAtHUDTargets = SugarLinear;
static FColor FollowWorldVisualization = CoralRedGamma;
static FColor LookAtWorldVisualization = RegalGamma;
static FColor LookAtTargetVector = RegalLinear;
static FColor LookAtDesiredTargetBox = CoralRedLinear;
static FColor BlackEyeCameraFrustum = SugarGamma;
static FColor LookAtPedestal = CoralRedGamma;

void UBlackEyeCameraSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
#if WITH_EDITOR && UE_ENABLE_DEBUG_DRAWING
    DebugDrawDelegate = FDebugDrawDelegate::CreateUObject(this, &UBlackEyeCameraSubsystem::DrawDebugUI);

    UDebugDrawService::Register(TEXT("BlackEyeCameras"), DebugDrawDelegate);
    ShowBlackEyeCameraFlag = IConsoleManager::Get().FindConsoleVariable(TEXT("ShowFlag.BlackEyeCameras"));
#endif
}

void UBlackEyeCameraSubsystem::Tick(float DeltaTime)
{
#if WITH_EDITOR && UE_ENABLE_DEBUG_DRAWING
    
    bool ShowBlackEyeCameras;
    ensure(ShowBlackEyeCameraFlag);
    ShowBlackEyeCameraFlag->GetValue(ShowBlackEyeCameras);
    if (!ShowBlackEyeCameras) return;

    UWorld* World = GetWorld();
    bool IsPlayingSession = GEditor->IsPlayingSessionInEditor();
    bool IsInPIEWorld = World && World->IsGameWorld();

    ABlackEyeCameraRigBase* ActiveCamera = nullptr;
    APlayerController* PC = Cast<APlayerController>(World->GetFirstPlayerController());
    if (PC && PC->PlayerCameraManager)
    {
        AActor* ViewTarget = PC->PlayerCameraManager->GetViewTarget();
        ActiveCamera = Cast<ABlackEyeCameraRigBase>(ViewTarget);

        if (ActiveCamera)
        {
            UCameraComponent* CamComponent = ActiveCamera->GetCamera().Get();

#if ENGINE_MINOR_VERSION < 5
            // TODO: Drawing these gizmos causes the camera viewport to shift when constrain aspect ratio is checked 
            // (https://www.reddit.com/r/unrealengine/comments/17v0l4n/does_anyone_know_why_drawing_a_debug_trace_line/) and is a UE 5.3 bug.

            if (CamComponent && CamComponent->bConstrainAspectRatio)
            {
                return;
            }
#endif

            UFollowComponent* FollowComponent = Cast<UFollowComponent>(ActiveCamera->GetComponentByClass(UFollowComponent::StaticClass()));
            if (FollowComponent != nullptr)
            {
                DrawFollowComponentVisualization(FollowComponent, World, CamComponent);
            }

            ULookAtComponent* LookAtComp = Cast<ULookAtComponent>(ActiveCamera->GetComponentByClass(ULookAtComponent::StaticClass()));
            if (LookAtComp)
            {
                DrawLookAtComponentVisualization(LookAtComp, World, CamComponent);
            }
        }
    }
    else if (!IsInPIEWorld)
    {
        for (TActorIterator<ABlackEyeCameraRigBase> ActorItr(GetWorld()); ActorItr; ++ActorItr)
        {
            ABlackEyeCameraRigBase* CamActor = *ActorItr;
            if (CamActor->IsHiddenEd()) continue;

            UCameraComponent* Camera = CamActor->GetCamera().Get();

            bool IsActiveInAnyViewport = false;
            const TArray<FLevelEditorViewportClient*>& Viewports = GEditor->GetLevelViewportClients();
            for (int i = 0; i < Viewports.Num(); ++i)
            {
                IsActiveInAnyViewport = Viewports[i]->IsActorLocked(CamActor);
                if (IsActiveInAnyViewport) break;
            }

            if (Camera && (CamActor->IsSelected() || IsActiveInAnyViewport))
            {
                UFollowComponent* FollowComponent = Cast<UFollowComponent>(CamActor->GetComponentByClass(UFollowComponent::StaticClass()));
                if (FollowComponent != nullptr)
                {
                    DrawFollowComponentVisualization(FollowComponent, World, Camera);
                }

                ULookAtComponent* LookAtComp = Cast<ULookAtComponent>(CamActor->GetComponentByClass(ULookAtComponent::StaticClass()));
                if (LookAtComp)
                {
                    DrawLookAtComponentVisualization(LookAtComp, World, Camera);
                }
            }
        }
    }

    ECameraFrustumDebugDrawMode FrustumDebugDrawMode = FBlackEyeCVars::ShowCameraFrustums();
    if (FrustumDebugDrawMode != ECameraFrustumDebugDrawMode::CFDDM_None)
    {
        for (TActorIterator<ABlackEyeCameraRigBase> ActorItr(GetWorld()); ActorItr; ++ActorItr)
        {
            ABlackEyeCameraRigBase* CamActor = *ActorItr;
            if (CamActor->IsHiddenEd()) continue;

            if (CamActor && ((ActiveCamera == CamActor && FrustumDebugDrawMode == ECameraFrustumDebugDrawMode::CFDDM_ActiveOnly)
                || FrustumDebugDrawMode == ECameraFrustumDebugDrawMode::CFDDM_AllFrustum))
            {
                UCameraComponent* Camera = CamActor->GetCamera().Get();
                if (Camera)
                {
                    float CameraAspectRatio = Camera->AspectRatio;
                    FIntPoint virtualViewportSize(1920, (int)(1080.f * CameraAspectRatio));


                    FSceneViewProjectionData CameraProjectionData = FBlackEyeUtilities::GetViewProjectionData(Camera,
                                                                                                                Camera->GetComponentLocation(),
                                                                                                                Camera->GetComponentRotation(),
                                                                                                                virtualViewportSize);

                    FMatrix ViewProjectionMatrix = CameraProjectionData.ComputeViewProjectionMatrix();
                    FBlackEyeUtilities::DrawDebugFrustum(World, ViewProjectionMatrix, 10.f, 1000.f, CameraAspectRatio, BlackEyeCameraFrustum);
                }
            }
        }
    }
#endif //WITH_EDITOR && UE_ENABLE_DEBUG_DRAWING
}

TStatId UBlackEyeCameraSubsystem::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UBlackEyeCameraSubsystem, STATGROUP_Tickables);
}

#if WITH_EDITOR && UE_ENABLE_DEBUG_DRAWING
void UBlackEyeCameraSubsystem::DrawDebugUI(UCanvas* Canvas, APlayerController* Controller)
{
    float DPIScale = Canvas->GetDPIScale();
    FVector2D Min = FVector2D::ZeroVector;
    FVector2D Size = FVector2D(Canvas->SizeX, Canvas->SizeY);

    Min /= DPIScale;
    Size /= DPIScale;

    bool DrawGuides = false;

    // Use the world from the debug service and not the subsystems as we can get calls from many different viewports
    ensure(Canvas->SceneView);
    const FSceneView* SceneView = Canvas->SceneView;
    ensure(SceneView->Family->Scene);
    UWorld* World = SceneView->Family->Scene->GetWorld();
    if (World)
    {
        // We're playing in this viewport so do runtime drawing
        APlayerController* PC = Cast<APlayerController>(World->GetFirstPlayerController());
        if (PC && PC->PlayerCameraManager)
        {
            // Do runtime HUD
            AActor* ViewTarget = PC->PlayerCameraManager->GetViewTarget();

            FString ActorName = ViewTarget->GetActorNameOrLabel();
            FString Text = FString::Printf(TEXT("Active Camera: `%s`"), *ActorName);

            const UFont* Font = GEngine->GetSmallFont();

            int32 TextX, TextY;
            Font->GetStringHeightAndWidth(*Text, TextY, TextX);

            const FColor OldDrawColor = Canvas->DrawColor;
            Canvas->SetDrawColor(FColor::White);
            Canvas->DrawText(Font, Text, Canvas->SizeX - (TextX + 25.f), TextY + 30.f);
            Canvas->SetDrawColor(OldDrawColor);

            if (ABlackEyeCameraRigBase* RigActor = Cast<ABlackEyeCameraRigBase>(ViewTarget))
            {
                ULocalPlayer* const LP = PC ? PC->GetLocalPlayer() : nullptr;
                if (!LP) return;

                FViewport* Viewport = LP->ViewportClient->Viewport;
                const UCameraComponent* Camera = RigActor->GetCamera().Get();
                if (!Camera) return;

                FCanvas* CanvasRef = Canvas->Canvas;

#if ENGINE_MINOR_VERSION < 5
                if (Camera->bConstrainAspectRatio)
                {
                    FString ConstrainWarning = FString(TEXT("Black Eye Camera Subsystem cannot draw world gizmos when the camera uses constrain aspect ratio due bug in UE 5.3/5.4!"));

                    int32 WarningX, WarningY;
                    Font->GetStringHeightAndWidth(*ConstrainWarning, WarningY, WarningX);

                    Canvas->SetDrawColor(FColor::Yellow);
                    Canvas->DrawText(Font, ConstrainWarning, Canvas->SizeX - (WarningX + 25.f), WarningY + TextY + 30.f);
                    Canvas->SetDrawColor(OldDrawColor);
                }
#endif

                UFollowComponent* FollowComponent = Cast<UFollowComponent>(RigActor->GetComponentByClass(UFollowComponent::StaticClass()));
                if (FollowComponent)
                {
                    DrawFollowComponentDebugHUD(CanvasRef, World, Camera, FollowComponent);
                }

                ULookAtComponent* LookAtComponent = Cast<ULookAtComponent>(RigActor->GetComponentByClass(ULookAtComponent::StaticClass()));
                if (LookAtComponent)
                {
                    DrawLookAtComponentDebugHUD(Canvas, World, Camera, LookAtComponent);
                    DrawGuides = true;
                }
            }
        }
        else if (!World->IsGameWorld())
        {
            if (FBlackEyeCVars::ShowCameraNames())
            {
                const UFont* Font = GEngine->GetSmallFont();
                const FColor OldDrawColor = Canvas->DrawColor;
                Canvas->SetDrawColor(FColor::White);

                // Draw camera rig names in viewport
                for (TActorIterator<ABlackEyeCameraRigBase> ActorItr(World); ActorItr; ++ActorItr)
                {
                    if (SceneView->ViewActor == *ActorItr || (*ActorItr)->IsHiddenEd())
                    {
                        continue;
                    }

                    const ABlackEyeCameraRigBase* RigActor = *ActorItr;
                    if (const UCameraComponent* Camera = RigActor->GetCamera().Get())
                    {
                        FVector ScreenPosition = Canvas->Project(Camera->GetComponentLocation());
                        // Ignore cameras behind us
                        if (ScreenPosition.Z > 0.f)
                        {
                            FString ActorName = RigActor->GetActorNameOrLabel();
                            FString Text = FString::Printf(TEXT("%s"), *ActorName);

                            int32 TextX, TextY;
                            Font->GetStringHeightAndWidth(*Text, TextY, TextX);
                            Canvas->DrawText(Font, Text, ScreenPosition.X + TextX / 2, ScreenPosition.Y);
                        }
                    }
                }
                Canvas->SetDrawColor(OldDrawColor);
            }

            //Draw HUD components in edit mode if active
            const ABlackEyeCameraRigBase* RigActor = Cast<ABlackEyeCameraRigBase>(SceneView->ViewActor);
            if (SceneView->ViewActor && RigActor)
            {
                const UCameraComponent* Camera = RigActor->GetCamera().Get();
                if (!Camera) return;

                UFollowComponent* FollowComponent = Cast<UFollowComponent>(RigActor->GetComponentByClass(UFollowComponent::StaticClass()));
                if (FollowComponent)
                {
                    DrawFollowComponentDebugHUD(Canvas->Canvas, World, Camera, FollowComponent);
                }

                ULookAtComponent* LookAtComponent = Cast<ULookAtComponent>(RigActor->GetComponentByClass(ULookAtComponent::StaticClass()));
                if (LookAtComponent)
                {
                    DrawLookAtComponentDebugHUD(Canvas, World, Camera, LookAtComponent);
                    DrawGuides = true;
                }
            }
        }
    }

    // Draw our rule of 3rds guides
    if (DrawGuides)
    {
        FColor GuideColour = FColor::Silver;
        FVector2D OneThirdViewport = Size * 1.f / 3.f;
        ::DrawDebugCanvas2DLine(Canvas, Min + FVector2D(0.f, OneThirdViewport.Y), Min + FVector2D(Size.X, OneThirdViewport.Y), GuideColour);
        ::DrawDebugCanvas2DLine(Canvas, Min + FVector2D(0.f, Size.Y - OneThirdViewport.Y), Min + FVector2D(Size.X, Size.Y - OneThirdViewport.Y), GuideColour);
        ::DrawDebugCanvas2DLine(Canvas, Min + FVector2D(OneThirdViewport.X, 0.f), Min + FVector2D(OneThirdViewport.X, Size.Y), GuideColour);
        ::DrawDebugCanvas2DLine(Canvas, Min + FVector2D(Size.X - OneThirdViewport.X, 0.f), Min + FVector2D(Size.X - OneThirdViewport.X, Size.Y), GuideColour);
    }
}

void UBlackEyeCameraSubsystem::DrawFollowComponentDebugHUD(FCanvas* Canvas, UWorld* World, const UCameraComponent* Camera, UFollowComponent* CamRigComp)
{
    //Do nothing in this for HUD. For now.
    
}

void UBlackEyeCameraSubsystem::DrawLookAtComponentDebugHUD(UCanvas* Canvas, UWorld* World, const UCameraComponent* Camera, ULookAtComponent* LookAtComp)
{
    if (!LookAtComp->CameraExistsInActor()) return;

    FVector LensPosition = Camera->GetComponentLocation();
    FRotator LensRotation = Camera->GetComponentRotation();

    float DPIScale =  Canvas->GetDPIScale();

    FIntPoint ViewportPosition = FIntPoint::ZeroValue;
    FIntPoint ViewportSize = FIntPoint(Canvas->SizeX, Canvas->SizeY);


    float AspectRatio = Camera->bConstrainAspectRatio ? Camera->AspectRatio : (float)ViewportSize.X / (float)ViewportSize.Y;

    FVector2D ResolvedTargetScreenPosition;
    bool HasValidTarget = false;

    UCameraComponent* CamComponent = LookAtComp->RootActorComponent->GetCamera().Get();
    FSceneViewProjectionData ViewProjectionData = FBlackEyeUtilities::GetViewProjectionData(CamComponent, 
                                                                                            LensPosition,
                                                                                            LensRotation,
                                                                                            ViewportSize, 
                                                                                            AspectRatio, 
                                                                                            Camera->FieldOfView);
    FIntRect ViewRect = ViewProjectionData.GetConstrainedViewRect();

    FVector2D Min = ViewRect.Min;
    FVector2D Size = ViewRect.Size();

    Min /= DPIScale; 
    Size /= DPIScale;

    // Draw our target
    switch (LookAtComp->TargetResolveType)
    {
    case EViewTargetResolveModes::VTRM_ScreenCenter:
    {
        FBox2D TargetBounds;
        FBox WorldBounds;
        HasValidTarget = LookAtComp->GetTargetGroupViewportBoundingBox(ViewProjectionData, TargetBounds, WorldBounds);
        if (HasValidTarget)
        {
            // HACK: Not sure why this is such an issue here but the view projection data not matching what we calculate inside of the component vs visualizeres.
            // But only when playing in editor.
            float AspectRatioRatio = World->IsPlayInEditor() ? AspectRatio / Camera->AspectRatio : 1.f;

            FVector2D TargetBoundsSize = TargetBounds.GetSize();
            TargetBoundsSize /= AspectRatioRatio;

            FVector2D TargetBoundsCenter = TargetBounds.GetCenter();
            TargetBoundsCenter.Y = 1.f - TargetBoundsCenter.Y;

            TargetBounds.Min = Min + (TargetBoundsCenter - TargetBoundsSize * 0.5f) * Size;
            TargetBounds.Max = Min + (TargetBoundsCenter + TargetBoundsSize * 0.5f) * Size;

           // UE_LOG(LogTemp, Display, TEXT("(Cam Subsystem[%.3f]): %.3f x %.3f"), aspectRatioRatio, targetBoundsSize.X, targetBoundsSize.Y);

            ::DrawDebugCanvas2DBox(Canvas, TargetBounds, LookAtHUDTargets);
            ResolvedTargetScreenPosition = TargetBounds.GetCenter();
        }
    }
    break;

    case EViewTargetResolveModes::VTRM_WorldCenter:
        {
            HasValidTarget = true;
            FVector LookAtPoint = LensPosition + LookAtComp->DesiredLookAtVectorWorld;
            FBlackEyeMath::ProjectWorldToViewport(LookAtPoint, ViewProjectionData.ComputeViewProjectionMatrix(), ResolvedTargetScreenPosition);

            ResolvedTargetScreenPosition.Y = 1.f - ResolvedTargetScreenPosition.Y;
            ResolvedTargetScreenPosition *= Size;
            ResolvedTargetScreenPosition += Min;
        }
        break;
    default:
        //Do nothing in HUD for these modes
        break;
    }

    // Draw center of target
    const float kScreenBoundsPointSize = 2.5f;

    FVector2D ResolvedScreenPointBoxSize = FVector2D::One() * kScreenBoundsPointSize * DPIScale;
    FBox2D targetBox(ResolvedTargetScreenPosition - ResolvedScreenPointBoxSize * 0.5f, ResolvedTargetScreenPosition + ResolvedScreenPointBoxSize * 0.5f);

    if (HasValidTarget)
    {
        ::DrawDebugCanvas2DBox(Canvas, targetBox, LookAtTargetVector);
    }

    // Draw our screen target position

    FBox2D TargetScreenPosition = LookAtComp->GetTargetViewportBox();
    TargetScreenPosition.Min.Y = 1.f - TargetScreenPosition.Min.Y;
    TargetScreenPosition.Max.Y = 1.f - TargetScreenPosition.Max.Y;

    FVector2D DeadZoneScreenSize = TargetScreenPosition.GetSize() * Size;
    if (DeadZoneScreenSize.SquaredLength() < 1.f)
    {
        DeadZoneScreenSize = 2.f * FVector2D::One();
    }

    FVector2D DeadZoneScreenCenter = TargetScreenPosition.GetCenter() * Size;;
    targetBox.Min = Min + DeadZoneScreenCenter - DeadZoneScreenSize * 0.5f;
    targetBox.Max = Min + DeadZoneScreenCenter + DeadZoneScreenSize * 0.5f;

    ::DrawDebugCanvas2DBox(Canvas, targetBox, LookAtDesiredTargetBox.ReinterpretAsLinear());
}

void UBlackEyeCameraSubsystem::DrawFollowComponentVisualization(class UFollowComponent* Follow, UWorld* World, class UCameraComponent* Camera)
{
    bool DrawWorldGizmos = true;

#if ENGINE_MINOR_VERSION < 5
    DrawWorldGizmos = !Camera->bConstrainAspectRatio;
#endif

    if (!DrawWorldGizmos) return;

    FVector FollowComponentPosition = Follow->GetComponentLocation();
    FVector FollowResolvedTargetPosition = Follow->TargetResolvedPosition;
    FVector FollowTargetDesiredPosition = Follow->DesiredPosition;

    FQuat FollowComponentRotation = Follow->GetComponentRotation().Quaternion();
    const UActorComponent* FollowTargetComponent = Follow->Target.Target.GetComponent(nullptr);

    if (Follow->DesiredRotation.AngularDistance(FollowComponentRotation) > FMath::DegreesToRadians(0.1f))
    {
        ::DrawDebugCoordinateSystem(World, FollowComponentPosition, FollowComponentRotation.Rotator(), 30.f);// , false, -1.f, ESceneDepthPriorityGroup::SDPG_Foreground);
        ::DrawDebugCoordinateSystem(World, FollowTargetDesiredPosition, Follow->DesiredRotation.Rotator(), 30);// , false, -1.f, ESceneDepthPriorityGroup::SDPG_Foreground);
    }

    //if (FVector::Dist(FollowComponentPosition, FollowTargetDesiredPosition) > Follow->FollowDwellRadius)
    {
        ::DrawDebugSphere(World, FollowTargetDesiredPosition, Follow->FollowDwellRadius, 8, FollowWorldVisualization);
    }

    if (FVector::DistSquared(FollowComponentPosition, FollowTargetDesiredPosition) > 1.f)
    {
        ::DrawDebugLine(World, FollowComponentPosition, FollowTargetDesiredPosition, FollowWorldVisualization);// , false, -1.f, ESceneDepthPriorityGroup::SDPG_Foreground);
    }

    if (Follow->bUseMultipleTargets && Follow->Targets.Num() > 1)
    {
        for (int i = 0; i < Follow->Targets.Num(); ++i)
        {
            if (!Follow->Targets[i].IsValid()) continue;

            FVector TargetPosition;
            Follow->Targets[i].GetTargetPosition(TargetPosition);

            ::DrawDebugLine(World, FollowResolvedTargetPosition, TargetPosition, FollowWorldVisualization);// , false, -1.f, ESceneDepthPriorityGroup::SDPG_Foreground);
        }
    }
}

void UBlackEyeCameraSubsystem::DrawLookAtComponentVisualization(class ULookAtComponent* LookAt, UWorld* World, class UCameraComponent* Camera)
{
    FBox OverlappedBox;
    bool DrawWorldGizmos = true;

#if ENGINE_MINOR_VERSION < 5
    DrawWorldGizmos = !Camera->bConstrainAspectRatio;
#endif

    if (!DrawWorldGizmos) return;

    switch (LookAt->TargetResolveType)
    {
    // World center gets encapsulating box + individual boxes
    case EViewTargetResolveModes::VTRM_WorldCenter:

        if (LookAt->GetTargetGroupBoundingVolume(OverlappedBox))
        {
            ::DrawDebugBox(World, OverlappedBox.GetCenter(), OverlappedBox.GetExtent(), LookAtWorldVisualization, false, -1.f, ESceneDepthPriorityGroup::SDPG_Foreground);
        }
        // Fall thru deliberate
    case EViewTargetResolveModes::VTRM_ScreenCenter:
        for (int i = 0; i < LookAt->LookAtTargets.Num(); ++i)
        {
            if (i > 1 && LookAt->TargetCombineMode == EViewTargeCombineModes::VTCM_First) break;
            if (!LookAt->LookAtTargets[i].IsValid()) continue;

            FBox TargetBox;
            if (LookAt->LookAtTargets[i].GetTargetBounds(TargetBox))
            {
                ::DrawDebugBox(World, TargetBox.GetCenter(), TargetBox.GetExtent(), LookAtWorldVisualization, false, -1.f, ESceneDepthPriorityGroup::SDPG_Foreground);
            }
        }
        break;
    }

    if (!FMath::IsNearlyZero(LookAt->CameraPlateDistance) || !FMath::IsNearlyZero(LookAt->CameraPedestalHeight))
    {
        FVector LookAtLocation = LookAt->GetComponentLocation();
        const FTransform& LookAtRotation = LookAt->GetComponentTransform();
        FVector ArmForwardOffset = LookAt->GetForwardVector() * LookAt->CameraPlateDistance;
        FVector ArmVerticalOffset = LookAt->GetUpVector() * LookAt->CameraPedestalHeight;
        ::DrawDebugLine(World, LookAtLocation, LookAtLocation + ArmForwardOffset, LookAtPedestal);
        ::DrawDebugLine(World, LookAtLocation + ArmForwardOffset, LookAtLocation + ArmForwardOffset + ArmVerticalOffset, LookAtPedestal);
    }
}
#endif // WITH_EDITOR && UE_ENABLE_DEBUG_DRAWING