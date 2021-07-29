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

#include "CoreMinimal.h"
#include "Manager/TimelineManagerDecorator.h"

#include "ConfiguredTimeline.generated.h"

/**
 * This struct to create Configured Timeline and ease Timeline instantiation.
 * This allows to associated a Timeline Name to a class.
 */
USTRUCT(BlueprintType)
struct NANSTIMELINESYSTEMUE4_API FConfiguredTimeline
{
	GENERATED_BODY()

	/**
	 * This allows to retrieve easily a timeline.
	 * This is used by the SConfiguredTimelinePin as a combobox.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NansTimeline")
	FName Name;

	/** The Configured Timeline class */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NansTimeline")
	TSubclassOf<UNTimelineManagerDecorator> TimelineClass;
};
