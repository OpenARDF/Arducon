#include <cstdlib>
#include <iostream>

#include "../Software/AtmelStudio7/Arducon/Arducon/SchedulerLogic.h"

namespace
{
constexpr time_t minimumEpoch = 1609459200;

void fail(const char* expression, const char* file, int line)
{
	std::cerr << file << ":" << line << ": assertion failed: " << expression << '\n';
	std::exit(1);
}

#define CHECK(expression) do { if(!(expression)) { fail(#expression, __FILE__, __LINE__); } } while(false)

ArduconScheduleConfig_t schedule(time_t currentEpoch, time_t startEpoch, time_t finishEpoch, uint8_t useRtc, uint8_t disabled)
{
	ArduconScheduleConfig_t config = {};
	config.current_epoch = currentEpoch;
	config.start_epoch = startEpoch;
	config.finish_epoch = finishEpoch;
	config.minimum_epoch = minimumEpoch;
	config.use_rtc_for_startstop = useRtc;
	config.transmissions_disabled = disabled;
	return config;
}

ArduconScheduleState_t classify(time_t currentEpoch, time_t startEpoch, time_t finishEpoch, uint8_t useRtc, uint8_t disabled)
{
	ArduconScheduleConfig_t config = schedule(currentEpoch, startEpoch, finishEpoch, useRtc, disabled);
	return arduconClassifySchedule(&config);
}

void checkScheduleClassification()
{
	CHECK(classify(minimumEpoch + 100, minimumEpoch + 200, minimumEpoch + 500, 1, 1) == ARDUCON_SCHEDULE_WAITING_FOR_START);
	CHECK(classify(minimumEpoch + 100, minimumEpoch + 200, minimumEpoch + 500, 0, 1) == ARDUCON_SCHEDULE_WILL_NEVER_RUN);
	CHECK(classify(minimumEpoch + 200, minimumEpoch + 200, minimumEpoch + 500, 1, 1) == ARDUCON_SCHEDULE_WAITING_FOR_START);
	CHECK(classify(minimumEpoch + 201, minimumEpoch + 200, minimumEpoch + 500, 1, 1) == ARDUCON_SCHEDULE_DID_NOT_START);
	CHECK(classify(minimumEpoch + 201, minimumEpoch + 200, minimumEpoch + 500, 1, 0) == ARDUCON_SCHEDULE_EVENT_IN_PROGRESS);
	CHECK(classify(minimumEpoch + 500, minimumEpoch + 200, minimumEpoch + 500, 1, 0) == ARDUCON_SCHEDULE_EVENT_IN_PROGRESS);
	CHECK(classify(minimumEpoch + 501, minimumEpoch + 200, minimumEpoch + 500, 1, 0) == ARDUCON_SCHEDULE_CONFIGURATION_ERROR);
	CHECK(classify(minimumEpoch + 100, minimumEpoch + 200, minimumEpoch + 200, 1, 1) == ARDUCON_SCHEDULE_CONFIGURATION_ERROR);
	CHECK(arduconClassifySchedule(nullptr) == ARDUCON_SCHEDULE_CONFIGURATION_ERROR);
}

void checkStartAndFinishBoundaries()
{
	const time_t start = minimumEpoch + 200;
	const time_t finish = minimumEpoch + 500;

	CHECK(!arduconShouldStartScheduledEvent(start - 1, start, finish, 0));
	CHECK(arduconShouldStartScheduledEvent(start, start, finish, 0));
	CHECK(arduconShouldStartScheduledEvent(finish - 1, start, finish, 0));
	CHECK(!arduconShouldStartScheduledEvent(finish, start, finish, 0));
	CHECK(!arduconShouldStartScheduledEvent(start, start, finish, 1));

	CHECK(!arduconShouldFinishScheduledEvent(finish - 1, finish));
	CHECK(arduconShouldFinishScheduledEvent(finish, finish));
	CHECK(arduconShouldFinishScheduledEvent(finish + 1, finish));
}

void checkRadioPrePower()
{
	const time_t start = minimumEpoch + 200;

	CHECK(!arduconShouldPrePowerRadio(start - 6, start, 5, 0));
	CHECK(arduconShouldPrePowerRadio(start - 5, start, 5, 0));
	CHECK(arduconShouldPrePowerRadio(start, start, 5, 0));
	CHECK(!arduconShouldPrePowerRadio(start, start, 5, 1));
}

void checkFoxSlotLogic()
{
	CHECK(arduconCurrentFoxShouldTransmit(1, 0, 1, 0));
	CHECK(arduconCurrentFoxShouldTransmit(5, 3, 3, 0));
	CHECK(!arduconCurrentFoxShouldTransmit(5, 4, 3, 0));
	CHECK(arduconCurrentFoxShouldTransmit(5, 10, 3, 7));

	CHECK(arduconScheduledFoxCounter(0, 300, 60, 5) == 1);
	CHECK(arduconScheduledFoxCounter(59, 300, 60, 5) == 1);
	CHECK(arduconScheduledFoxCounter(60, 300, 60, 5) == 2);
	CHECK(arduconScheduledFoxCounter(299, 300, 60, 5) == 5);
	CHECK(arduconScheduledFoxCounter(300, 300, 60, 5) == 1);
	CHECK(arduconScheduledFoxCounter(125, 300, 60, 5) == 3);
	CHECK(arduconScheduledFoxCounter(125, 0, 60, 5) == 1);
}
}

int main()
{
	checkScheduleClassification();
	checkStartAndFinishBoundaries();
	checkRadioPrePower();
	checkFoxSlotLogic();

	std::cout << "Scheduler logic unit tests passed." << '\n';
	return 0;
}
