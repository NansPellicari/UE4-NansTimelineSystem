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
#include "Event.h"

class FNTimelineManager;

enum class ENTimelineEvent : uint8
{
	/**
	* This event should be triggered right before an event is attached to a timeline.
	* This way it can offer the opportunity to avoid an event to be attached in a dedicated logic.
	*/
	BeforeAttached,

	/**
	* This event should be triggered right after an event is attached to a timeline.
	* This way it can offer the opportunity to create some stats or else...
	*/
	AfterAttached,

	/** This event should be triggered when an event expired. */
	Expired,

	/**  */
	Start,
};

DECLARE_MULTICAST_DELEGATE_FourParams(
	FNTimelineEventDelegate,
	TSharedPtr<FNEvent> /** Event */,
	const ENTimelineEvent& /** EventName */,
	const float& /** ExpiredTime */,
	const int32& /** Index */
);

/**
 * @see NTimelineInterface
 */
class NANSTIMELINESYSTEMCORE_API FNTimeline final
{
	/** Only the FNTimelineManager can tick a timeline object */
	friend class FNTimelineManager;
public:
	FNTimeline();

	/**
	 * @param InLabel - (optional) The name of this timeline. If not provided it creates a name with a static incremented value.
	 */
	FNTimeline(const FName& InLabel);

	/** Empty events array */
	~FNTimeline();

	/**
	 * Attached an event to this timeline. 
	 * It creates a FEventTuple and calls BeforeOnAttached() to checks if it can be attached
	 * and AfterOnAttached() for any custom usages
	 *
	 * @param Event - The event you want to put in the timeline stream
	 */
	bool Attached(const TSharedPtr<FNEvent>& Event);

	/**
	 * Same as Attached(TSharedPtr<FNEvent> Event) but for a collection of objects.
	 *
	 * @see FNTimeline::Attached(TSharedPtr<FNEvent> Event)
	 */
	void Attached(const TArray<TSharedPtr<FNEvent>>& EventsCollection);

	/**
	* This is the value required by a timer manager to know
	* the tick frequency for this timeline (given by NTimelineManager).
	* The NotifyTick use this method to add time on CurrentTime
	* at each call.
	*/
	float GetTickInterval() const;

	/** Retrieve the current time since this timeline exists and play */
	float GetCurrentTime() const;

	/**
	 * Give a name to this timeline
	 * @param InLabel - The name
	 */
	void SetLabel(const FName& InLabel);

	/** Return the actual name */
	FName GetLabel() const;

	/**
	 * This completely reset every events.
	 * It should be used with caution.
	 */
	void Clear();

	/**
	* Offer the opportunities to save data in a binary object.
	* @param Ar - Archive where we need to save or load data.
	*/
	void Archive(FArchive& Ar);

	/** @returns Get the list of all events saved in this timeline */
	TArray<TSharedPtr<FNEvent>> GetEvents() const;

	/**
	* Get an event by its UID
	* @returns the event found or invalid TSharedPtr
	*/
	TSharedPtr<FNEvent> GetEvent(const FString& InUID) const;

private:
	/** The name of this timeline */
	FName Label;

	/**
	 * Tick interval, should be set by its manager
	 */
	float TickInterval = 1.f;

	/**
	 * It is computed internally in the NotifyTick() method.
	 * In every tick it adds GetTickInterval() return.
	 */
	float CurrentTime = 0.f;

	/**
	 * Defined the tick interval for this timeline.
	 * This should be called only by its friend INTimelineManagerInterface
	 * or a decorator to maintain consistency with its manager.
	 * @param InTickInterval - Time in secs
	 */
	void SetTickInterval(const float& InTickInterval);

	/**
	 * Should be used only for serialization because it is internally computed with NotifyTick()
	 * @param InCurrentTime - Time in secs
	 */
	void SetCurrentTime(const float& InCurrentTime);

	/** This is used to managed and event when it expires */
	void OnExpired(const TSharedPtr<FNEvent>& Event, const float& ExpiredTime, const int32& Index) const;

	/** This is used to managed and event when it starts */
	void StartEvent(const TSharedPtr<FNEvent>& Event, const int32& Index) const;

	/**
	 * This manages to notify every events saved in this timeline with the new time added.
	 * It uses internally GetTickInterval() to increment time.
	 */
	void NotifyTick();

	/** Collection of each Events attached to the timeline. */
	TArray<TSharedPtr<FNEvent>> Events;

	/** @see FTimeline() */
	FNTimelineEventDelegate EventChanged;

	// This is global to avoid similar generated name when retrieved from archive
	static int32 Counter;
};
