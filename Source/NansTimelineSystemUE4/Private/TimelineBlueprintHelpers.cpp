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

#include "TimelineBlueprintHelpers.h"

#include "Engine/GameInstance.h"
#include "Event/TimelineEventDecorator.h"
#include "Manager/TimelineManagerBaseDecorator.h"
#include "NansTimelineSystemCore/Public/Timeline.h"
#include "TimelineGameInstance.h"

UNTimelineManagerBaseDecorator* UNTimelineBlueprintHelpers::CreateNewTimeline(
	UObject* WorldContextObject, TSubclassOf<UNTimelineManagerBaseDecorator> Class, FName Name)
{
	UNTimelineManagerBaseDecorator* Object =
		UNTimelineManagerBaseDecorator::CreateObject<UNTimelineManagerBaseDecorator>(WorldContextObject, Class, Name);
	return Object;
}

UNTimelineManagerBaseDecorator* UNTimelineBlueprintHelpers::GetTimeline(UObject* WorldContextObject, FConfiguredTimeline Timeline)
{
	UGameInstance* GI = WorldContextObject->GetWorld()->GetGameInstance();
	if (!GI->GetClass()->ImplementsInterface(UNTimelineGameInstance::StaticClass()))
	{
		UE_LOG(LogTemp, Error, TEXT("Your game instance should implements INTimelineGameInstance"));
		return nullptr;
	}

	return INTimelineGameInstance::Execute_GetTimeline(GI, Timeline);
}
