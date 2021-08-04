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

class INEvent
{
public:
	/** Default dtor */
	virtual ~INEvent() = default;
	/** It indicates if the event expired. */
	virtual bool IsExpired() const = 0;

	/** Returns the localTime since the events has been attached (+ delay if > 0) to a timeline */
	virtual float GetLocalTime() const = 0;

	/** The time relative to the timeline this event has been attached to. */
	virtual float GetAttachedTime() const = 0;

	/** The time relative to the timeline this event has been attached to + its start delay. */
	virtual float GetStartedAt() const = 0;

	/** The duration this event should live */
	virtual float GetDuration() const = 0;

	/** The delay before this event starts */
	virtual float GetDelay() const = 0;

	/** Retrieve the unique ID generated or given in ctor */
	virtual FString GetUID() const = 0;

	/**
	* The time relative to the timeline this event has been expired,
	* should return -1 if this event has no duration.
	*/
	virtual float GetExpiredTime() const = 0;

	/** Getter for Label */
	virtual FName GetEventLabel() const = 0;

	/** Timeline use this to know if this event can be attached on. */
	virtual bool IsAttachable() const = 0;

	/**
	* A setter for the label.
	*
	* @param InEventLabel - A name to identify easily the event
	*/
	virtual void SetEventLabel(const FName& InEventLabel) = 0;
};

/**
 * An interface to manage events which can be attached to a timeline.
 */
class NANSTIMELINESYSTEMCORE_API FNEvent : public INEvent
{
public:
	FNEvent();
	/** Ctor to gives directly a name for this event and an Id (optional). */
	FNEvent(const FName& InLabel, const FString& InUId = FString(""));

	// ~ Begin INEvent overrides
	virtual bool IsExpired() const override;
	virtual float GetLocalTime() const override;
	virtual float GetAttachedTime() const override;
	virtual float GetStartedAt() const override;
	virtual float GetDuration() const override;
	virtual float GetDelay() const override;
	virtual FString GetUID() const override;
	virtual float GetExpiredTime() const override;
	virtual FName GetEventLabel() const override;
	virtual bool IsAttachable() const override;
	virtual void SetEventLabel(const FName& InEventLabel) override;
	// ~ End INEvent overrides

	/**
	 * This should be used only on serialization process
	 *
	 * @param InUId - Should be a unique FString, see ctor
	 */
	virtual void SetUID(const FString& InUId);

	/** Set the time this event is attached to timeline, should be used only by a FNTimeline. */
	virtual void SetAttachedTime(const float& InLocalTime);

	/**
	 * This can be useful to avoid an Event to be attached to a timeline.
	 * @see FNTimeline::Attached()
	 * @see ENTimelineEvent::BeforeAttached
	 * @param bInIsAttachable - boolean to defined is attachable capability
	 */
	virtual void SetAttachable(const bool& bInIsAttachable);

	/**
	 * Set the expired time for this event.
	 * It is called by the FNTimeline.
	 * @param InLocalTime - the time relative to the timeline
	 */
	virtual void SetExpiredTime(const float& InLocalTime);

	/**
	 * This setter should be carefully used,
	 * all the computation time should be calculated internally
	 * with the NotifyAddTime().
	 *
	 * @param InLocalTime - Time in secs
	 */
	virtual void SetLocalTime(const float& InLocalTime);
	/**
	 * A setter for the duration.
	 *
	 * @param InDuration - Time in secs
	 */
	virtual void SetDuration(const float& InDuration);

	/**
	 * A setter for the delay.
	 *
	 * @param InDelay - Time in secs
	 */
	virtual void SetDelay(const float& InDelay);

	/**
	 * This should be used only by NTimeline or serialization.
	 *
	 * @param StartTime - Time in secs
	 */
	virtual void Start(const float& StartTime);

	/**
	 * This can stop the event and make it expires to its next tick.
	 */
	virtual void Stop();

	/**
	 * Increments LocalTime
	 * @param NewTime - in Milliseconds
	 */
	virtual void NotifyAddTime(const float& NewTime);

	/** This should reset all data */
	virtual void Clear();

	virtual void Archive(FArchive& Ar);

	friend bool operator==(const TSharedPtr<FNEvent>& Event, const FString& InUId)
	{
		return InUId == Event->GetUID();
	}

protected:
	// TODO add this possibility later
	// TArray<uint8> ExtraData
	FName Label = NAME_None;
	float AttachedTime = 0.f;
	float LocalTime = 0.f;
	float StartedAt = -1.f;
	float ExpiredTime = -1.f;
	float Duration = 0.f;
	float Delay = 0.f;
	FString UId;
	bool bActivated = false;
	bool bIsAttachable = true;
};
