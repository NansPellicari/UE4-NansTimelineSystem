#include "CoreMinimal.h"
#include "Engine/DebugCameraController.h"
#include "Engine/Engine.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/EngineTypes.h"
#include "EngineGlobals.h"
#include "Misc/AutomationTest.h"
#include "NansUE4TestsHelpers/Public/Helpers/Assertions.h"
#include "NansUE4TestsHelpers/Public/Helpers/TestWorld.h"
#include "NansUE4TestsHelpers/Public/Mock/MockObject.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Runtime/Engine/Public/Tests/AutomationCommon.h"
#include "Runtime/Engine/Public/Tickable.h"
#include "Specs/TimelineManagerFake.h"
#include "Specs/TimelineManagerTickableOnPauseFake.h"
#include "TimerManager.h"

/**
 * For an unknown reason, I can't create a Spec with World ticking,
 * frame doesn't increase. But a SIMPLE_AUTOMATION_TEST works.
 */
// clang-format off
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTimelineTestCounterIncrease,
"Nans.TimelineSystem.UE4.TimelineManager.Test.ShouldTickThanksToWorldTimerManager", EAutomationTestFlags::EditorContext |
EAutomationTestFlags::EngineFilter)
// clang-format on
bool FTimelineTestCounterIncrease::RunTest(const FString& Parameters)
{
	const double StartTime = FPlatformTime::Seconds();
	UWorld* World = NTestWorld::CreateAndPlay(EWorldType::Game);
	FTimerManager& TimerManager = World->GetTimerManager();
	NTimelineManagerFake* TimelineManager = new NTimelineManagerFake();
	World->GetTimerManager().SetTimer(
		TimelineManager->TimerHandle, TimelineManager->TimerDelegate, TimelineManager->GetTickInterval(), true);

	// Begin test
	{
		TimelineManager->Play();
		// clang-format off
		TEST_TRUE(TEST_TEXT_FN_DETAILS("Handle should be valid after calling SetTimer"), TimelineManager->TimerHandle.IsValid());
		TEST_TRUE(TEST_TEXT_FN_DETAILS("TimerExists called with a pending timer"), TimerManager.TimerExists(TimelineManager->TimerHandle));
		TEST_TRUE(TEST_TEXT_FN_DETAILS("IsTimerActive called with a pending timer"), TimerManager.IsTimerActive(TimelineManager->TimerHandle));
		TEST_FALSE(TEST_TEXT_FN_DETAILS("IsTimerPaused called with a pending timer"), TimerManager.IsTimerPaused(TimelineManager->TimerHandle));
		TEST_TRUE(TEST_TEXT_FN_DETAILS("GetTimerRate called with a pending timer"), (TimerManager.GetTimerRate(TimelineManager->TimerHandle) == TimelineManager->GetTickInterval()));
		TEST_TRUE(TEST_TEXT_FN_DETAILS("GetTimerElapsed called with a pending timer"), (TimerManager.GetTimerElapsed(TimelineManager->TimerHandle) == 0.f));
		TEST_TRUE(TEST_TEXT_FN_DETAILS("GetTimerRemaining called with a pending timer"), (TimerManager.GetTimerRemaining(TimelineManager->TimerHandle) == TimelineManager->GetTickInterval()));
		// clang-format on

		// small tick to move the timer from the pending list to the active list, the timer will start counting time after this tick
		NTestWorld::Tick(World, KINDA_SMALL_NUMBER);
		NTestWorld::Tick(World);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has been called 1"), TimelineManager->Counter, 1.f);
	}
	// End test

	NTestWorld::Destroy(World);
	TimelineManager = nullptr;
	UE_LOG(LogTemp, Display, TEXT("2- Test run on %f ms"), (FPlatformTime::Seconds() - StartTime) * 1000.f);
	return true;
}

// clang-format off
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTimelineTestPauseAndPlay, "Nans.TimelineSystem.UE4.TimelineManager.Test.ShouldPauseAndPlay",
EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
// clang-format on
bool FTimelineTestPauseAndPlay::RunTest(const FString& Parameters)
{
	const double StartTime = FPlatformTime::Seconds();
	UWorld* World = NTestWorld::CreateAndPlay(EWorldType::Game);
	FTimerManager& TimerManager = World->GetTimerManager();
	NTimelineManagerFake* TimelineManager = new NTimelineManagerFake();
	World->GetTimerManager().SetTimer(
		TimelineManager->TimerHandle, TimelineManager->TimerDelegate, TimelineManager->GetTickInterval(), true);

	// Begin test
	{
		TimelineManager->Play();
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has not been called"), TimelineManager->Counter, 0.f);
		NTestWorld::Tick(World, KINDA_SMALL_NUMBER);
		NTestWorld::Tick(World);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has been called 1"), TimelineManager->Counter, 1.f);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has been called 2"), TimelineManager->Counter, 2.f);
		TimelineManager->Pause();
		NTestWorld::Tick(World);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has been called still 2"), TimelineManager->Counter, 2.f);
		TimelineManager->Play();
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has been called still 3"), TimelineManager->Counter, 3.f);
	}
	// End test

	NTestWorld::Destroy(World);
	TimelineManager = nullptr;
	UE_LOG(LogTemp, Display, TEXT("2- Test run on %f ms"), (FPlatformTime::Seconds() - StartTime) * 1000.f);
	return true;
}

// clang-format off
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTimelineTestTimerManagerPauseAndPlay,
"Nans.TimelineSystem.UE4.TimelineManager.Test.ShouldPauseAndPlayCauseOfTimerManager", EAutomationTestFlags::EditorContext |
EAutomationTestFlags::EngineFilter)
// clang-format on
bool FTimelineTestTimerManagerPauseAndPlay::RunTest(const FString& Parameters)
{
	const double StartTime = FPlatformTime::Seconds();
	UWorld* World = NTestWorld::CreateAndPlay(EWorldType::Game);
	FTimerManager& TimerManager = World->GetTimerManager();
	NTimelineManagerFake* TimelineManager = new NTimelineManagerFake();
	World->GetTimerManager().SetTimer(
		TimelineManager->TimerHandle, TimelineManager->TimerDelegate, TimelineManager->GetTickInterval(), true);

	// Begin test
	{
		TimelineManager->Play();
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has not been called"), TimelineManager->Counter, 0.f);
		NTestWorld::Tick(World, KINDA_SMALL_NUMBER);
		NTestWorld::Tick(World);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has been called 1"), TimelineManager->Counter, 1.f);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has been called 2"), TimelineManager->Counter, 2.f);
		TimerManager.PauseTimer(TimelineManager->TimerHandle);
		NTestWorld::Tick(World);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has been called still 2"), TimelineManager->Counter, 2.f);
		TimerManager.UnPauseTimer(TimelineManager->TimerHandle);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has been called still 3"), TimelineManager->Counter, 3.f);
	}
	// End test

	NTestWorld::Destroy(World);
	TimelineManager = nullptr;
	UE_LOG(LogTemp, Display, TEXT("2- Test run on %f ms"), (FPlatformTime::Seconds() - StartTime) * 1000.f);
	return true;
}

// clang-format off
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTimelineTestTimerManagerGamePause,
"Nans.TimelineSystem.UE4.TimelineManager.Test.SetGamePauseShouldNotInfluenceTimerManagerAttachedToWorld",
EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
// clang-format on
bool FTimelineTestTimerManagerGamePause::RunTest(const FString& Parameters)
{
	const double StartTime = FPlatformTime::Seconds();
	UWorld* World = NTestWorld::CreateAndPlay(EWorldType::Game, true);
	UGameInstance* GI = World->GetGameInstance();
	TimelineManagerTickableOnPauseFake* TimelineManager = new TimelineManagerTickableOnPauseFake(World);
	UMockObject* MockObject = NewObject<UMockObject>(World, FName("MyMockObject"), EObjectFlags::RF_MarkAsRootSet);
	MockObject->SetMyWorld(World);

	// Begin test
	{
		TimelineManager->Play();
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has not been called"), TimelineManager->Counter, 0.f);
		NTestWorld::Tick(World, KINDA_SMALL_NUMBER);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has been called 1"), TimelineManager->Counter, 1.f);
		UGameplayStatics::SetGamePaused(MockObject, true);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has been called 2"), TimelineManager->Counter, 2.f);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has been called 3"), TimelineManager->Counter, 3.f);
		UGameplayStatics::SetGamePaused(MockObject, false);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has been called 4"), TimelineManager->Counter, 4.f);
		TimelineManager->Pause();
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has been called still 4"), TimelineManager->Counter, 4.f);
	}
	// End test

	NTestWorld::Destroy(World);
	MockObject->ClearFlags(EObjectFlags::RF_Transient);
	MockObject->RemoveFromRoot();
	TimelineManager = nullptr;
	UE_LOG(LogTemp, Display, TEXT("2- Test run on %f ms"), (FPlatformTime::Seconds() - StartTime) * 1000.f);
	return true;
}

// clang-format off
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FTimelineTestTimerManagerGamePauseWithController,
"Nans.TimelineSystem.UE4.TimelineManager.Test.SetGamePauseInfluenceTimerManagerAttachedToWorldSTimerManager",
EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
// clang-format on
bool FTimelineTestTimerManagerGamePauseWithController::RunTest(const FString& Parameters)
{
	const double StartTime = FPlatformTime::Seconds();
	UWorld* World = NTestWorld::CreateAndPlay(EWorldType::Game, true);
	FTimerManager& TimerManager = World->GetTimerManager();

	UMockObject* MockObject = NewObject<UMockObject>(World, FName("MyMockObject"), EObjectFlags::RF_MarkAsRootSet);
	MockObject->SetMyWorld(World);
	APlayerController* PC = World->GetFirstPlayerController();

	if (PC == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("No controller has been founded"));
		return false;
	}

	NTimelineManagerFake* TimelineManager = new NTimelineManagerFake();
	TimerManager.SetTimer(TimelineManager->TimerHandle, TimelineManager->TimerDelegate, TimelineManager->GetTickInterval(), true);
	CollectGarbage(EObjectFlags::RF_NoFlags);
	// Begin test
	{
		TimelineManager->Play();
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has not been called"), TimelineManager->Counter, 0.f);
		NTestWorld::Tick(World, KINDA_SMALL_NUMBER);
		NTestWorld::Tick(World);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager has been called 1"), TimelineManager->Counter, 1.f);
		UGameplayStatics::SetGamePaused(MockObject, true);
		NTestWorld::Tick(World);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager Should stick to 1 count after making game paused"),
			TimelineManager->Counter,
			1.f);
		UGameplayStatics::SetGamePaused(MockObject, false);
		NTestWorld::Tick(World);
		TEST_EQ(TEST_TEXT_FN_DETAILS("Timeline manager tick once more after stopping game paused"), TimelineManager->Counter, 2.f);
	}
	// End test

	MockObject->ClearFlags(EObjectFlags::RF_Transient);
	MockObject->RemoveFromRoot();
	NTestWorld::Destroy(World);
	TimelineManager = nullptr;
	UE_LOG(LogTemp, Display, TEXT("2- Test run on %f ms"), (FPlatformTime::Seconds() - StartTime) * 1000.f);
	return true;
}
