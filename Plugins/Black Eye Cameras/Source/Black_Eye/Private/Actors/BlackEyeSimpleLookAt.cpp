// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.


#include "Actors/BlackEyeSimpleLookAt.h"


ABlackEyeSimpleLookAt::ABlackEyeSimpleLookAt()
	: ABlackEyeCameraRigBase()
{
	Follow = CreateDefaultSubobject<UFollowComponent>(TEXT("Follow"));
	SetRootComponent(Follow);

	LookAt = CreateDefaultSubobject<ULookAtComponent>(TEXT("LookAt"));
	LookAt->AttachToComponent(Follow, FAttachmentTransformRules::KeepRelativeTransform);
}