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

class NTimelineEventBase;
class NTimelineManagerAbstract;

/**
 * \ingroup Core
 * Its goal is to saved events and place them in time.
 */
class NANSTIMELINESYSTEMCORE_API NTimeline
{
	friend class NTimelineManagerAbstract;

	/**
	 * 0: Event object
	 * 1: attached time
	 * 2: delay
	 * 3: duration
	 * 4: label
	 * 5: expired time
	 */
	using FEventTuple = TTuple<TSharedPtr<NTimelineEventBase>, float, const float, const float, const FName, float>;

public:
	virtual ~NTimeline();
	void Attached(TSharedPtr<NTimelineEventBase> Event);
	void Attached(TArray<TSharedPtr<NTimelineEventBase>> EventsCollection);
	float GetCurrentTime();
	void SetLabel(FName _Label);

protected:
	/** The name of this timeline */
	FName Label;

	/**
	 * This is by this object we can control time behavior (play, stop, etc,..).
	 * But it also garanty to tick the timeline when itself is ticked.
	 */
	TSharedPtr<NTimelineManagerAbstract> TimerManager;

	// In case of specialisation need to avoid the attach process in some cases
	virtual bool BeforeOnAttached(TSharedPtr<NTimelineEventBase> Event, const float AttachedTime)
	{
		return true;
	};

	// If needed to make some stats, analytics, trigger error,...
	virtual void AfterOnAttached(TSharedPtr<NTimelineEventBase> Event, const float AttachedTime) {}

	/**
	 * This is the value required by a timer to know
	 * the tick frequency for this timeline.
	 * The NotifyTick use this method to add time on CurrentTime
	 * at each call.
	 */
	virtual const float& GetTickInterval() const
	{
		static const float TickInterval = 1.0f;
		return TickInterval;
	}

	/**
	 * Use Event SharedPtr with caution, it's pointer is reset just after this method is called.
	 * @warning the Event should be used internally only to avoid nullptr reference
	 */
	virtual void OnExpired(TSharedPtr<NTimelineEventBase> Event, const float AttachedTime) {}

	const TArray<NTimeline::FEventTuple> GetEvents() const;

	/**
	 * This completely reset every events.
	 * Is should be used internally,
	 * a timeline should manage its duration itself.
	 */
	void Clear();

private:
	/**
	 * It's computed internaly in the NotifyTick() method.
	 * In every tick it adds GetTickInterval() return.
	 */
	float CurrentTime = 0.f;

	/**
	 * A Timeline can't exists with a timer.
	 * To have it in the constructor, it garanties the behavior consistancy between them.
	 *
	 * @param TimerManager - Timer which manage all time behavior (tick, pause, stop, play,...)
	 */
	NTimeline(NTimelineManagerAbstract* TimerManager);

	/**
	 * This managed to notify every events saved in this timeline with the new time added.
	 * This method is private to ensure it is called only by the Timer friend class.
	 * It allows to ensure the consistency between timer & timeline behavior.
	 *
	 * > Important notes:
	 * It never destroys an event tuple, this to keep a trace of what happens.
	 * It could be used for stats or user feedbacks for instance.
	 * But inside the tuple, the shared pointer is destroyed to avoid memory leaks.
	 */
	void NotifyTick();

	/** Collection of each Events attached to the timeline */
	TArray<FEventTuple> Events;
};
