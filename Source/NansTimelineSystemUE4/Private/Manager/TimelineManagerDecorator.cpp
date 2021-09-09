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

#include "Manager/TimelineManagerDecorator.h"

#include "Event/EventView.h"
#include "NansTimelineSystemUE4.h"

FString EnumToString(const ENTimelineEvent& Value)
{
	static const UEnum* TypeEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ENTimelineEvent"));
	return TypeEnum->GetNameStringByIndex(static_cast<int32>(Value));
}

UNTimelineManagerDecorator::UNTimelineManagerDecorator() : FNTimelineManager() {}

void UNTimelineManagerDecorator::Init(const float& InTickInterval, const FName& InLabel)
{
	ensureMsgf(GetWorld() != nullptr, TEXT("A UNTimelineManagerDecorator need a world to live"));
	TickInterval = InTickInterval;
	FNTimelineManager::Init(InTickInterval, InLabel);

	OnEventChanged().AddUObject(this, &UNTimelineManagerDecorator::OnEventChangedDelegate);
}

void UNTimelineManagerDecorator::Pause()
{
	FNTimelineManager::Pause();
}

void UNTimelineManagerDecorator::Play()
{
	if (StartedAt == -1.f)
	{
		StartedAt = FDateTime::Now();
	}
	FNTimelineManager::Play();
}

void UNTimelineManagerDecorator::Stop()
{
	FNTimelineManager::Stop();
}

struct FParamsEventChanged
{
	float InLocalTime = -1.f;
	UWorld* InWorld = nullptr;
	APlayerController* InPlayer = nullptr;
};

void UNTimelineManagerDecorator::OnEventChangedDelegate(TSharedPtr<INEvent> Event,
	const ENTimelineEvent& EventName, const float& LocalTime, const int32& Index)
{
	UNEventView* EventView = EventViews.FindRef(Event->GetUID());
	if (!ensure(IsValid(EventView)))
	{
		return;
	}

	OnBPEventChanged(EventView, LocalTime);

	FString FuncName = FString::Printf(TEXT("On%s"), *EnumToString(EventName));
	UFunction* Func = EventView->FindFunction(FName(FuncName));

	if (Func != nullptr)
	{
		bool bSupported = true;
		FParamsEventChanged Param;
		Param.InLocalTime = LocalTime;
		Param.InWorld = GetWorldChecked(bSupported);
		Param.InPlayer = Param.InWorld->GetFirstPlayerController();

		UE_DEBUG_LOG(
			LogTimelineSystem, Display, TEXT("FuncName \"%s\" for event \"%s\" will be called at %f secs"), *FuncName,
			*EventView->GetEventLabel().ToString(), LocalTime
		);
		EventView->ProcessEvent(Func, &Param);
	}
	else
	{
		UE_DEBUG_LOG(
			LogTimelineSystem, Warning, TEXT("FuncName \"%s\" for event \"%s\" not exists"), *FuncName,
			*EventView->GetEventLabel().ToString()
		);
	}

	if (EventName == ENTimelineEvent::Expired)
	{
		ExpiredEventViews.Add(Event->GetUID(), EventView);
		EventViews.Remove(Event->GetUID());
	}
}

TArray<UNEventView*> UNTimelineManagerDecorator::GetEventViews() const
{
	TArray<UNEventView*> EventRecords;
	EventViews.GenerateValueArray(EventRecords);
	return EventRecords;
}

TArray<UNEventView*> UNTimelineManagerDecorator::GetExpiredEventViews() const
{
	TArray<UNEventView*> EventRecords;
	ExpiredEventViews.GenerateValueArray(EventRecords);
	return EventRecords;
}

UNEventView* UNTimelineManagerDecorator::GetEventView(const FString& InUID) const
{
	return EventViews.FindRef(InUID);
}

float UNTimelineManagerDecorator::GetCurrentTime() const
{
	check(GetTimeline().IsValid());
	return GetTimeline()->GetCurrentTime();
}

FName UNTimelineManagerDecorator::GetLabel() const
{
	check(GetTimeline().IsValid());
	return GetTimeline()->GetLabel();
}

void UNTimelineManagerDecorator::SetLabel(const FName& Name)
{
	check(GetTimeline().IsValid());
	GetTimeline()->SetLabel(Name);
}

UNEventView* UNTimelineManagerDecorator::CreateAndAddNewEvent(FName InName, float InDuration, float InDelay,
	TSubclassOf<UNEventView> InClass)
{
	UClass* ChildClass;
	if (InClass)
	{
		ChildClass = *InClass;
	}
	else
	{
		ChildClass = UNEventView::StaticClass();
	}

	const TSharedPtr<INEvent> Object = CreateNewEvent(InName, InDuration, InDelay);
	if (!Object.IsValid()) return nullptr;

	UNEventView* EventView = NewObject<UNEventView>(this, ChildClass);
	EventView->Init(Object, GetCurrentTime(), GetWorld(), GetWorld()->GetFirstPlayerController());
	EventViews.Add(Object->GetUID(), EventView);

	GetTimeline()->Attached(Object);
	return EventView;
}

void UNTimelineManagerDecorator::Clear()
{
	EventViews.Empty();
	ExpiredEventViews.Empty();
	FNTimelineManager::Clear();
}

void UNTimelineManagerDecorator::Serialize(FArchive& Ar)
{
	// Thanks to the UE4 serializing system, this will serialize all uproperty with "SaveGame"
	Super::Serialize(Ar);
	Archive(Ar);

	int32 NumEntries = 0;
	int32 NumExpiredEntries = 0;

	if (Ar.IsSaving())
	{
		NumEntries = EventViews.Num();
		NumExpiredEntries = ExpiredEventViews.Num();
	}

	Ar << NumEntries;
	Ar << NumExpiredEntries;

	if (Ar.IsSaving() && NumEntries > 0)
	{
		for (TTuple<FString, UNEventView*> Pair : EventViews)
		{
			Ar << Pair.Key;
			FString PathClass = Pair.Value->GetClass()->GetPathName();
			Ar << PathClass;
			Pair.Value->Serialize(Ar);
		}
	}

	if (Ar.IsSaving() && NumExpiredEntries > 0)
	{
		for (TTuple<FString, UNEventView*> Pair : ExpiredEventViews)
		{
			Ar << Pair.Key;
			FString PathClass = Pair.Value->GetClass()->GetPathName();
			Ar << PathClass;
			Pair.Value->Serialize(Ar);
		}
	}

	if (Ar.IsLoading() && NumEntries > 0)
	{
		for (int32 I = 0; I < NumEntries; I ++)
		{
			FString Id, PathClass;
			Ar << Id;
			Ar << PathClass;

			TSharedPtr<INEvent> Event = Timeline->GetEvent(Id);

			if (ensureMsgf(
				Event.IsValid(), TEXT("Event with Uid (\"%s\") can't be retrieved during serialization."), *Id
			))
			{
				UClass* Class = ConstructorHelpersInternal::FindOrLoadClass(PathClass, UNEventView::StaticClass());
				UNEventView* Object = NewObject<UNEventView>(this, Class);
				Object->Serialize(Ar);
				Object->Init(Event, GetCurrentTime(), GetWorld(), GetWorld()->GetFirstPlayerController());
				EventViews.Emplace(Id, Object);
			}
		}
	}

	if (Ar.IsLoading() && NumExpiredEntries > 0)
	{
		for (int32 I = 0; I < NumExpiredEntries; I ++)
		{
			FString Id, PathClass;
			Ar << Id;
			Ar << PathClass;

			TSharedPtr<INEvent> Event = Timeline->GetExpiredEvent(Id);

			if (ensureMsgf(
				Event.IsValid(), TEXT("Event with Uid (\"%s\") can't be retrieved during serialization."), *Id
			))
			{
				UClass* Class = ConstructorHelpersInternal::FindOrLoadClass(PathClass, UNEventView::StaticClass());
				UNEventView* Object = NewObject<UNEventView>(this, Class);
				Object->Serialize(Ar);
				Object->Init(Event, GetCurrentTime(), GetWorld(), GetWorld()->GetFirstPlayerController());
				ExpiredEventViews.Emplace(Id, Object);
			}
		}
	}
}

void UNTimelineManagerDecorator::BeginDestroy()
{
	OnEventChanged().RemoveAll(this);
	Clear();
	Super::BeginDestroy();
}
