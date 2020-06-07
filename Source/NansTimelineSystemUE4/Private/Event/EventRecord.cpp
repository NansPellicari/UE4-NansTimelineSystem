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

#include "Event/EventRecord.h"

#include "TimelineDecorator.h"

void FNEventRecord::Serialize(FArchive& Ar, UNTimelineDecorator* Timeline)
{
	if (Ar.IsSaving() && Event != nullptr)
	{
		Event->Serialize(Ar);
	}
	if (Ar.IsLoading() && EventClassName != FString(""))
	{
		UClass* Class = FindObject<UClass>(ANY_PACKAGE, *EventClassName);
		Event = Timeline->CreateNewEvent(Class, Label);
		Event->Serialize(Ar);
	}
}