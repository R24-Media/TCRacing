// Copyright 2024 Black Eye Technologies, Ltd. All Rights Reserved.

#include "Black_Eye.h"

#include "ShowFlags.h"

#define LOCTEXT_NAMESPACE "FBlack_EyeModule"

void FBlack_EyeModule::StartupModule()
{
	FEngineShowFlags::RegisterCustomShowFlag(TEXT("BlackEyeCameras"), true, SFG_Normal, FText::FromString("Black Eye Cameras"));
}

void FBlack_EyeModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBlack_EyeModule, Black_Eye)
