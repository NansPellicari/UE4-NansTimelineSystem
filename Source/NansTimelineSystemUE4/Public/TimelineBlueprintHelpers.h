// Copyright 2020-present Nans Pellicari (nans.pellicari@gmail.com).
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include "Attribute/ConfiguredTimeline.h"
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "TimelineBlueprintHelpers.generated.h"

class UNTimelineManagerBaseDecorator;
class UNTimelineEventDecorator;

/**
 * A simple Blueprint Library class to manage Timeline creation.
 */
UCLASS()
class NANSTIMELINESYSTEMUE4_API UNTimelineBlueprintHelpers : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	/**
	 * It allows you to create a new timeline manager on the flye with its embeded timeline.
	 * @note Prefer using the Configured Timeline which embed all the serialization functionnalities.
	 *
	 * @param WorldContextObject - This is as a Outer object for UNTimelineManagerBaseDecorator instanciation, it is implicitly
	 * provided by kismet library thanks to UFUNCTION meta data "WorldContext"
	 * @param Class - The class you want to instanciate
	 * @param Name - The name of your timeline
	 */
	// clang-format off
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", DisplayName = "Create a New NansTimeline", Keywords = "Timeline create"), Category = "NansTimeline")
	static UNTimelineManagerBaseDecorator* CreateNewTimeline(UObject* WorldContextObject, TSubclassOf<UNTimelineManagerBaseDecorator> Class, FName Name);
	// clang-format on

	/**
	 * This class is a pass-through for the INTimelineGameInstance::GetTimeline() method.
	 * It provides a standalone node to avoid getting the game instance in your BP graph.
	 *
	 * @param WorldContextObject - This is as a Outer object for UNTimelineManagerBaseDecorator instanciation, it is implicitly
	 * provided by kismet library thanks to UFUNCTION meta data "WorldContext"
	 * @param Timeline - To allow having a combobox of configured timelines
	 */
	// clang-format off
	UFUNCTION(BlueprintCallable, BlueprintPure, meta = (WorldContext = "WorldContextObject", DisplayName = "Get a NansTimeline by its configured name", Keywords = "Timeline get"), Category = "NansTimeline")
	static UNTimelineManagerBaseDecorator* GetTimeline(UObject* WorldContextObject, FConfiguredTimeline Timeline);
	// clang-format on
};
