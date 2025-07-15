// Copyright 2022 lumines_labs. All Rights Reserved.

#pragma once

#include "CoreUObject.h"
#include "Modules/ModuleManager.h"

#include "Styling/SlateStyle.h" //Icon

class FCinematographerEditorModule : public IModuleInterface
{

public:

	TSharedPtr<FSlateStyleSet> StyleSet; //Icon

	virtual void StartupModule() override;
	virtual void ShutdownModule() override;


};