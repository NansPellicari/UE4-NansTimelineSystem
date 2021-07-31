#include "CoreMinimal.h"
#include "GoogleTestApp.h"
#include "NansTimelineSystemCore/Public/Event.h"
#include "NansTimelineSystemCore/Public/Timeline.h"
#include "NansTimelineSystemCore/Public/TimelineManager.h"
#include "gtest/gtest.h"

#include <iostream>

class FNEventFake : public FNEvent
{
public:
	FName EventLabel;
	bool bIsExpired = false;

	FNEventFake(FName InLabel, float InDuration = 0.f, float InDelay = 0.f)
	{
		EventLabel = InLabel;
		Duration = InDuration;
		Delay = InDelay;
	}

	virtual bool IsExpired() const override
	{
		return bIsExpired || FNEvent::IsExpired();
	};

	virtual FName GetEventLabel() const override
	{
		return EventLabel;
	}

	void SetExpired()
	{
		bIsExpired = true;
	};
};

class NansTimelineSystemCoreTimelineTest : public ::testing::Test
{
protected:
	FNTimelineManager* Timer;
	TArray<TSharedPtr<FNEventFake>> Events;

	void SetUp() override
	{
		Timer = new FNTimelineManager();
		Events = {
			MakeShareable(new FNEventFake(FName("event 0"), 0)),
			MakeShareable(new FNEventFake(FName("event 1"), 2.f)),
			MakeShareable(new FNEventFake(FName("event 2"), 1.f, 2.f)),
			MakeShareable(new FNEventFake(FName("event 3"), 4.f)),
			MakeShareable(new FNEventFake(FName("event 4"), 1.f)),
		};
	}
};

TEST_F(NansTimelineSystemCoreTimelineTest, ICanCreateAndDestroyTimelineAndTimelineManager)
{
	FNTimelineManager* TimelineManager = new FNTimelineManager();
	delete TimelineManager;
}

TEST_F(NansTimelineSystemCoreTimelineTest, ShouldGetAValidTimelineAfterInstanciationAndATickInterval)
{
	ASSERT_TRUE(Timer->GetTimeline().IsValid());
	ASSERT_NE(Timer->GetTimeline()->GetTickInterval(), 0);
}

TEST_F(NansTimelineSystemCoreTimelineTest, ShouldIncrementItsCurrentTimeWhenTimerTickAndOnlyWhenIsPlaying)
{
	float TickInterval = Timer->GetTimeline()->GetTickInterval();
	ASSERT_TRUE(Timer->GetTimeline().IsValid());
	Timer->TimerTick();
	// just to be sure in case of a wrong incrementation computation
	Timer->TimerTick();
	EXPECT_EQ(Timer->GetTimeline()->GetCurrentTime(), 0);
	Timer->Play();
	EXPECT_EQ(Timer->GetState(), ENTimelineTimerState::Played);
	EXPECT_EQ(Timer->GetTimeline()->GetCurrentTime(), 0);
	Timer->TimerTick();
	EXPECT_EQ(Timer->GetTimeline()->GetCurrentTime(), TickInterval);
	Timer->TimerTick();
	EXPECT_EQ(Timer->GetTimeline()->GetCurrentTime(), 2 * TickInterval);
	Timer->TimerTick();
	EXPECT_EQ(Timer->GetTimeline()->GetCurrentTime(), 3 * TickInterval);
}

TEST_F(NansTimelineSystemCoreTimelineTest, ShouldManageCorrectlyDifferentEvents)
{
	ASSERT_TRUE(Timer->GetTimeline().IsValid());

	EXPECT_TRUE(Events[0].IsUnique());
	Timer->GetTimeline()->Attached({Events[0], Events[1]});
	EXPECT_FALSE(Events[0].IsUnique());
	EXPECT_FALSE(Events[1].IsUnique());
	Timer->Play();
	Timer->TimerTick(); // current time: 1 sec
	// Should start an finish in the same time as the "event4"
	Timer->GetTimeline()->Attached(Events[2]);
	Timer->GetTimeline()->Attached(Events[3]);
	Timer->TimerTick(); // 2 secs
	Timer->TimerTick(); // 3 secs
	Timer->GetTimeline()->Attached(Events[4]);
	EXPECT_FALSE(Events[0]->IsExpired());
	EXPECT_EQ(Events[1]->GetStartedAt(), 0);
	EXPECT_EQ(Events[1]->GetLocalTime(), 2.f);
	EXPECT_TRUE(Events[1]->IsExpired());
	// means it has been removed from the timeline events collection
	EXPECT_TRUE(Events[1].IsUnique());
	EXPECT_EQ(Events[2]->GetStartedAt(), 3.f);
	EXPECT_FALSE(Events[2]->IsExpired());
	EXPECT_FALSE(Events[2].IsUnique());
	Timer->TimerTick(); // 4 sec
	EXPECT_TRUE(Events[2]->IsExpired());
	EXPECT_TRUE(Events[2].IsUnique());
	EXPECT_TRUE(Events[4]->IsExpired());
	EXPECT_TRUE(Events[4].IsUnique());
	Timer->TimerTick(); // 5 sec
	EXPECT_TRUE(Events[3]->IsExpired());
	Timer->TimerTick(); // 6 sec

	// event 0
	EXPECT_EQ(Events[0]->GetLocalTime(), 6.f);
	EXPECT_EQ(Events[0]->GetStartedAt(), 0.f);
	// event 1
	EXPECT_EQ(Events[1]->GetLocalTime(), 2.f);
	EXPECT_EQ(Events[1]->GetStartedAt(), 0.f);
	// event 2
	EXPECT_EQ(Events[2]->GetLocalTime(), 1.f);
	EXPECT_EQ(Events[2]->GetStartedAt(), 3.f); // 2 sec delay + attached at 1 sec
	// event 3
	EXPECT_EQ(Events[3]->GetLocalTime(), 4.f);
	EXPECT_EQ(Events[3]->GetStartedAt(), 1.f);
	// event 4
	EXPECT_EQ(Events[4]->GetLocalTime(), 1.f);
	EXPECT_EQ(Events[4]->GetStartedAt(), 3.f);

	// this one is infinite
	EXPECT_FALSE(Events[0]->IsExpired());
}

TEST_F(NansTimelineSystemCoreTimelineTest, ShouldNotIncrementTimeWhenTimerIsPausedOrStopped)
{
	ASSERT_TRUE(Timer->GetTimeline().IsValid());

	Timer->Play();
	Timer->TimerTick();
	EXPECT_EQ(Timer->GetTimeline()->GetCurrentTime(), 1.f);
	Timer->TimerTick();
	EXPECT_EQ(Timer->GetTimeline()->GetCurrentTime(), 2.f);

	Timer->Pause();
	Timer->TimerTick(); // should not increment the timeline
	EXPECT_EQ(Timer->GetTimeline()->GetCurrentTime(), 2.f);
	Timer->TimerTick(); // dito
	EXPECT_EQ(Timer->GetTimeline()->GetCurrentTime(), 2.f);

	Timer->Play();
	Timer->TimerTick(); // Should increment
	EXPECT_EQ(Timer->GetTimeline()->GetCurrentTime(), 3.f);

	Timer->Stop();
	Timer->TimerTick(); // should not
	EXPECT_EQ(Timer->GetTimeline()->GetCurrentTime(), 0.f);
	Timer->TimerTick(); // dito
	EXPECT_EQ(Timer->GetTimeline()->GetCurrentTime(), 0.f);
}

TEST_F(NansTimelineSystemCoreTimelineTest, ShouldManageEventWhichHasBeenExpiredManually)
{
	ASSERT_TRUE(Timer->GetTimeline().IsValid());

	Timer->GetTimeline()->Attached(Events[0]);
	Timer->Play();
	Timer->TimerTick();

	EXPECT_FALSE(Events[0]->IsExpired());
	EXPECT_FALSE(Events[0].IsUnique());
	EXPECT_EQ(Events[0]->GetStartedAt(), 0);
	EXPECT_EQ(Events[0]->GetLocalTime(), 1.f);
	Events[0]->SetExpired();
	Timer->TimerTick();
	EXPECT_TRUE(Events[0]->IsExpired());
	EXPECT_TRUE(Events[0].IsUnique());
	EXPECT_EQ(Events[0]->GetStartedAt(), 0);
	EXPECT_NE(Events[0]->GetLocalTime(), 2.f);
}

TEST_F(NansTimelineSystemCoreTimelineTest, ShouldManageAHigherTickFrequency)
{
	Timer->Init(0.5f);
	ASSERT_TRUE(Timer->GetTimeline().IsValid());
	Timer->Play();
	Timer->TimerTick(); // should be 0.5sec
	EXPECT_EQ(Timer->GetTimeline()->GetCurrentTime(), 0.5f);
	Timer->TimerTick(); // should be 1sec
	EXPECT_EQ(Timer->GetTimeline()->GetCurrentTime(), 1.f);

	Timer->GetTimeline()->Attached(Events[1]);
	EXPECT_FALSE(Events[1]->IsExpired());
	Timer->TimerTick(); // should be 1.5sec
	Timer->TimerTick(); // should be 2sec
	EXPECT_FALSE(Events[1]->IsExpired());
	Timer->TimerTick(); // should be 2.5sec
	Timer->TimerTick(); // should be 3sec
	EXPECT_TRUE(Events[1]->IsExpired());
}

TEST_F(NansTimelineSystemCoreTimelineTest, ShouldTriggerAnEventWhenEventExpired)
{
	bool Test = false;
	FString UID = Events[1]->GetUID();
	Timer->OnEventChanged().AddLambda(
		[&Test, &UID](TSharedPtr<FNEvent> Event, const ENTimelineEvent& EventName, const float& ExpiredTime,
		const int32& Index)
		{
			if (EventName == ENTimelineEvent::Expired)
			{
				Test = Event->GetUID() == UID;
				EXPECT_EQ(ExpiredTime, 2.f);
			}
		}
	);

	Timer->Play();
	Timer->GetTimeline()->Attached(Events[1]);
	Timer->TimerTick();
	Timer->TimerTick();
	Timer->TimerTick();
	EXPECT_TRUE(Events[1]->IsExpired());
	EXPECT_TRUE(Test);
}

TEST_F(NansTimelineSystemCoreTimelineTest, ShouldTriggerAnEventWhenEventStart)
{
	bool Test = false;
	FString UID = Events[2]->GetUID();
	Timer->OnEventChanged().AddLambda(
		[&Test, &UID](TSharedPtr<FNEvent> Event, const ENTimelineEvent& EventName, const float& EventTime,
		const int32& Index)
		{
			if (EventName == ENTimelineEvent::Start && UID == Event->GetUID())
			{
				EXPECT_EQ(EventTime, 3.f);
				Test = true;
			}
		}
	);

	Timer->Play();
	Timer->TimerTick();
	EXPECT_EQ(Timer->GetTimeline()->GetCurrentTime(), 1.f);
	Timer->GetTimeline()->Attached(Events[2]);
	Timer->TimerTick();
	EXPECT_EQ(Timer->GetTimeline()->GetCurrentTime(), 2.f);
	Timer->TimerTick();
	Timer->TimerTick();
	EXPECT_EQ(Timer->GetTimeline()->GetCurrentTime(), 4.f);
	EXPECT_EQ(Events[2]->GetStartedAt(), 3.f);
	EXPECT_TRUE(Events[2]->IsExpired());
	EXPECT_TRUE(Test);
}
