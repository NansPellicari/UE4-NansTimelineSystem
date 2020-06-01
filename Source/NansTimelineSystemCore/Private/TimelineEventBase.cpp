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

#include "TimelineEventBase.h"

bool NTimelineEventBase::IsExpired() const
{
	return GetDuration() > 0 && GetLocalTime() >= GetDuration();
};

const float NTimelineEventBase::GetLocalTime() const
{
	return LocalTime;
}

const float NTimelineEventBase::GetStartedAt() const
{
	return StartedAt;
}

float NTimelineEventBase::GetDuration() const
{
	return Duration;
}
float NTimelineEventBase::GetDelay() const
{
	return Delay;
}

const FName NTimelineEventBase::GetEventLabel() const
{
	return Label;
}

const FString NTimelineEventBase::GetUID() const
{
	return Id;
}

void NTimelineEventBase::SetLocalTime(float _LocalTime)
{
	LocalTime = _LocalTime;
}

void NTimelineEventBase::SetStartedAt(float _StartedAt)
{
	StartedAt = _StartedAt;
}

void NTimelineEventBase::SetDuration(float _Duration)
{
	Duration = _Duration;
}

void NTimelineEventBase::SetDelay(float _Delay)
{
	Delay = _Delay;
}

void NTimelineEventBase::SetEventLabel(FName _EventLabel)
{
	Label = _EventLabel;
}

void NTimelineEventBase::Start(float StartTime)
{
	StartedAt = StartTime;
}

void NTimelineEventBase::NotifyAddTime(float NewTime)
{
	LocalTime += NewTime;
}

void NTimelineEventBase::Clear()
{
	Label = NAME_None;
	LocalTime = 0.f;
	StartedAt = -1.f;
	Duration = 0.f;
	Delay = 0.f;
}
