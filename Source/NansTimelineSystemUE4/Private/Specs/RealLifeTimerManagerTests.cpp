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

#include "CoreMinimal.h"
#include "Engine/DebugCameraController.h"
#include "Engine/Engine.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/EngineTypes.h"
#include "EngineGlobals.h"
#include "Manager/RealLifeTimelineManager.h"
#include "Misc/AutomationTest.h"
#include "NansUE4TestsHelpers/Public/Helpers/Assertions.h"
#include "NansUE4TestsHelpers/Public/Helpers/TestWorld.h"
#include "NansUE4TestsHelpers/Public/Mock/MockObject.h"
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformProcess.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Public/Tests/AutomationCommon.h"
#include "Serialization/BufferArchive.h"
#include "Specs/TimelineEventAdapterFake.h"
#include "TimerManager.h"

/**
 * For an unknown reason, I can't create a Spec with World ticking,
 * frame doesn't increase. But a SIMPLE_AUTOMATION_TEST works... o.O
 */
// clang-format off
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRealLifeTimelineManagerTest,
"Nans.TimelineSystem.UE4.RealLifeTimelineManager.Test.CanPauseAndPlayButShouldNotBeAffectedByGamePause", EAutomationTestFlags::EditorContext |
EAutomationTestFlags::EngineFilter)
// clang-format on
bool FRealLifeTimelineManagerTest::RunTest(const FString& Parameters)
{
	const double StartTime = FPlatformTime::Seconds();
	UWorld* World = NTestWorld::CreateAndPlay(EWorldType::Game, true);
	// RF_MarkAsRootSet to avoid deletion when GC passes
	UMockObject* MockObject = NewObject<UMockObject>(World, FName("MyMockObject"), EObjectFlags::RF_MarkAsRootSet);
	MockObject->SetMyWorld(World);
	UNRealLifeTimelineManager* TimelineManager = UNTimelineManagerBaseAdapter::CreateObject<UNRealLifeTimelineManager>(
		MockObject, FName("TestTimeline"), EObjectFlags::RF_MarkAsRootSet);

	// Begin test
	{
		FPlatformProcess::Sleep(1.1f);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has been called 1"), TimelineManager->GetTimelineTime(), 1.f);
		TimelineManager->Stop();
		FPlatformProcess::Sleep(1.1f);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager should never be stopped"), TimelineManager->GetTimelineTime(), 2.f);
		TimelineManager->Pause();
		FPlatformProcess::Sleep(1.1f);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager should never be paused"), TimelineManager->GetTimelineTime(), 3.f);
		NTestWorld::Tick(World);
		NTestWorld::Tick(World);
		NTestWorld::Tick(World);
		NTestWorld::Tick(World);
		NTestWorld::Tick(World);
		NTestWorld::Tick(World);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Should not be influenced by ticking if realtime doesn't increase"),
			TimelineManager->GetTimelineTime(),
			3.f);
	}
	// End test

	NTestWorld::Destroy(World);
	UE_LOG(LogTemp, Display, TEXT("2- Test run on %f ms"), (FPlatformTime::Seconds() - StartTime) * 1000.f);
	return true;
}

// clang-format off
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRealLifeTimelineManagerSerializationSameObjTest,
"Nans.TimelineSystem.UE4.RealLifeTimelineManager.Test.CanSerializeWithTheSameObjectInstance", EAutomationTestFlags::EditorContext |
EAutomationTestFlags::EngineFilter)
// clang-format on
bool FRealLifeTimelineManagerSerializationSameObjTest::RunTest(const FString& Parameters)
{
	const double StartTime = FPlatformTime::Seconds();
	UWorld* World = NTestWorld::CreateAndPlay(EWorldType::Game, true);
	// RF_MarkAsRootSet to avoid deletion when GC passes
	UMockObject* MockObject = NewObject<UMockObject>(World, FName("MyMockObject"), EObjectFlags::RF_MarkAsRootSet);
	MockObject->SetMyWorld(World);
	UNRealLifeTimelineManager* TimelineManager = UNTimelineManagerBaseAdapter::CreateObject<UNRealLifeTimelineManager>(
		MockObject, FName("TestTimeline"), EObjectFlags::RF_MarkAsRootSet);

	// Begin test
	{
		FPlatformProcess::Sleep(2.1f);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has been called 2"), TimelineManager->GetTimelineTime(), 2.f);
		FBufferArchive ToBinary;
		TimelineManager->Serialize(ToBinary);
		FPlatformProcess::Sleep(1.1f);
		NTestWorld::Tick(World);
		TimelineManager->Init(FName("ChangeLabel"));	// try to change label to checks if rewrite with the archive
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has been called 3"), TimelineManager->GetTimelineTime(), 3.f);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager label changed"), TimelineManager->GetLabel(), FName("ChangeLabel"));
		FMemoryReader FromBinary = FMemoryReader(ToBinary, true);
		FromBinary.Seek(0);
		TimelineManager->Serialize(FromBinary);
		TEST_EQ(
			TEST_TEXT_FN_DETAILS("Timeline manager label reload from archive"), TimelineManager->GetLabel(), FName("TestTimeline"));
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline should recover lost time since last serialization"),
			TimelineManager->GetTimelineTime(),
			3.f);
	}
	// End test

	NTestWorld::Destroy(World);
	UE_LOG(LogTemp, Display, TEXT("2- Test run on %f ms"), (FPlatformTime::Seconds() - StartTime) * 1000.f);
	return true;
}
// clang-format off
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRealLifeTimelineManagerSerializationDiffObjTest,
"Nans.TimelineSystem.UE4.RealLifeTimelineManager.Test.CanSerializeWithADifferentObjectInstance", EAutomationTestFlags::EditorContext |
EAutomationTestFlags::EngineFilter)
// clang-format on
bool FRealLifeTimelineManagerSerializationDiffObjTest::RunTest(const FString& Parameters)
{
	const double StartTime = FPlatformTime::Seconds();
	UWorld* World = NTestWorld::CreateAndPlay(EWorldType::Game, true);
	// RF_MarkAsRootSet to avoid deletion when GC passes
	UMockObject* MockObject = NewObject<UMockObject>(World, FName("MyMockObject"), EObjectFlags::RF_MarkAsRootSet);
	MockObject->SetMyWorld(World);
	UNRealLifeTimelineManager* TimelineManager = UNTimelineManagerBaseAdapter::CreateObject<UNRealLifeTimelineManager>(
		MockObject, FName("TestTimeline"), EObjectFlags::RF_MarkAsRootSet);

	// Begin test
	{
		FPlatformProcess::Sleep(2.1f);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has been called 2"), TimelineManager->GetTimelineTime(), 2.f);

		// Save in memory
		FBufferArchive ToBinary;
		TimelineManager->Serialize(ToBinary);
		TimelineManager->ConditionalBeginDestroy();
		UNRealLifeTimelineManager* NewTimelineManager = UNTimelineManagerBaseAdapter::CreateObject<UNRealLifeTimelineManager>(
			MockObject, FName("DiffTimelineLabel"), EObjectFlags::RF_MarkAsRootSet);
		FPlatformProcess::Sleep(1.1f);
		NTestWorld::Tick(World);
		TEST_EQ(
			TEST_TEXT_FN_DETAILS("New Timeline manager has been called 1 before load"), NewTimelineManager->GetTimelineTime(), 1.f);
		TEST_EQ(TEST_TEXT_FN_DETAILS("New Timeline add a different label from the saved one"),
			NewTimelineManager->GetLabel(),
			FName("DiffTimelineLabel"));

		// Load from memory
		FMemoryReader FromBinary = FMemoryReader(ToBinary, true);
		FromBinary.Seek(0);
		NewTimelineManager->Serialize(FromBinary);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager label reload from archive"),
			NewTimelineManager->GetLabel(),
			FName("TestTimeline"));
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline should recover lost time since last serialization"),
			NewTimelineManager->GetTimelineTime(),
			3.f);
	}
	// End test

	NTestWorld::Destroy(World);
	UE_LOG(LogTemp, Display, TEXT("2- Test run on %f ms"), (FPlatformTime::Seconds() - StartTime) * 1000.f);
	return true;
}

// clang-format off
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FRealLifeTimelineManagerEventTest,
"Nans.TimelineSystem.UE4.RealLifeTimelineManager.Test.CanAddEventsAndSaveThem", EAutomationTestFlags::EditorContext |
EAutomationTestFlags::EngineFilter)
// clang-format on
bool FRealLifeTimelineManagerEventTest::RunTest(const FString& Parameters)
{
	const double StartTime = FPlatformTime::Seconds();
	UWorld* World = NTestWorld::CreateAndPlay(EWorldType::Game, true);
	// RF_MarkAsRootSet to avoid deletion when GC passes
	UMockObject* MockObject = NewObject<UMockObject>(World, FName("MyMockObject"), EObjectFlags::RF_MarkAsRootSet);
	MockObject->SetMyWorld(World);
	UNRealLifeTimelineManager* TimelineManager = UNTimelineManagerBaseAdapter::CreateObject<UNRealLifeTimelineManager>(
		MockObject, FName("TestTimeline"), EObjectFlags::RF_MarkAsRootSet);

	// Begin test
	{
		TimelineManager->CreateAndAddNewEvent(UNTimelineEventAdapterFake::StaticClass(), NAME_None);
		FPlatformProcess::Sleep(1.1f);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Event live since 1 sec"), TimelineManager->GetEvents()[0].Event->GetLocalTime(), 1.f);
		TEST_EQ(TEST_TEXT_FN_DETAILS("There is 1 Event in collection"), TimelineManager->GetEvents().Num(), 1);
		TimelineManager->CreateAndAddNewEvent(UNTimelineEventAdapterFake::StaticClass(), FName("Ev2"));
		FPlatformProcess::Sleep(1.1f);
		NTestWorld::Tick(World);
		TimelineManager->CreateAndAddNewEvent(UNTimelineEventAdapterFake::StaticClass(), FName("Ev3"), 1.f);
		NTestWorld::Tick(World);
		// clang-format off
		TEST_EQ(TEST_TEXT_FN_DETAILS("There is 3 Events in collection"), TimelineManager->GetEvents().Num(), 3);
		TEST_EQ(TEST_TEXT_FN_DETAILS("1st event should have an automatic name"),TimelineManager->GetEvents()[0].Event->GetEventLabel(), FName("EventAdapter_1"));
		TEST_EQ(TEST_TEXT_FN_DETAILS("2nd event should have a choosen name"), TimelineManager->GetEvents()[1].Event->GetEventLabel(), FName("Ev2"));
		TEST_EQ(TEST_TEXT_FN_DETAILS("1st event live since 2 secs"), TimelineManager->GetEvents()[0].Event->GetLocalTime(), 2.f);
		TEST_EQ(TEST_TEXT_FN_DETAILS("2nd event live since 1 sec"), TimelineManager->GetEvents()[1].Event->GetLocalTime(), 1.f);
		TEST_EQ(TEST_TEXT_FN_DETAILS("3rd event live since 0 sec"), TimelineManager->GetEvents()[2].Event->GetLocalTime(), 0.f);
		TEST_FALSE(TEST_TEXT_FN_DETAILS("3rd event is not expired"), TimelineManager->GetEvents()[2].Event->IsExpired());
		// clang-format on

		// Save in memory
		FBufferArchive ToBinary;
		TimelineManager->Serialize(ToBinary);
		TimelineManager->ConditionalBeginDestroy();
		NTestWorld::Tick(World);

		UNRealLifeTimelineManager* NewTimelineManager = UNTimelineManagerBaseAdapter::CreateObject<UNRealLifeTimelineManager>(
			MockObject, FName("DiffTimelineLabel"), EObjectFlags::RF_MarkAsRootSet);

		// Load from memory
		FMemoryReader FromBinary = FMemoryReader(ToBinary, true);
		FromBinary.Seek(0);
		NewTimelineManager->Serialize(FromBinary);

		FPlatformProcess::Sleep(1.1f);
		NTestWorld::Tick(World);

		TEST_EQ(TEST_TEXT_FN_DETAILS("There is 3 Events in collection retrieving from serialized data"),
			NewTimelineManager->GetEvents().Num(),
			3);
		TEST_NOT_NULL(TEST_TEXT_FN_DETAILS("1st event should not be null"), NewTimelineManager->GetEvents()[0].Event);
		TEST_NOT_NULL(TEST_TEXT_FN_DETAILS("2nd event should not be null"), NewTimelineManager->GetEvents()[1].Event);
		TEST_EQ(TEST_TEXT_FN_DETAILS("1st event live since 3 secs"), NewTimelineManager->GetEvents()[0].Event->GetLocalTime(), 3.f);
		TEST_EQ(TEST_TEXT_FN_DETAILS("1st event get started at 0"), NewTimelineManager->GetEvents()[0].Event->GetStartedAt(), 0.f);
		TEST_EQ(TEST_TEXT_FN_DETAILS("2nd event live since 2sec"), NewTimelineManager->GetEvents()[1].Event->GetLocalTime(), 2.f);
		TEST_EQ(TEST_TEXT_FN_DETAILS("2nd event get started at 1"), NewTimelineManager->GetEvents()[1].Event->GetStartedAt(), 1.f);
		TEST_NULL(TEST_TEXT_FN_DETAILS("3rd event is null"), NewTimelineManager->GetEvents()[2].Event);
		TEST_GT(TEST_TEXT_FN_DETAILS("3rd event is expired"), NewTimelineManager->GetEvents()[2].ExpiredTime, 0.f);
	}
	// End test

	NTestWorld::Destroy(World);
	UE_LOG(LogTemp, Display, TEXT("2- Test run on %f ms"), (FPlatformTime::Seconds() - StartTime) * 1000.f);
	return true;
}