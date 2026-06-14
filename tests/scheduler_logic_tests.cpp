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
	config.event_days = 1;
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

void checkRTCGatePlans()
{
	const time_t start = minimumEpoch + 200;
	const time_t finish = minimumEpoch + 500;

	ArduconRTCGatePlan_t noRtc = arduconPlanRTCGate(start, start, finish, 1, 5, 1, 0, 0);
	CHECK(!noRtc.pre_power_radio);
	CHECK(!noRtc.start_event);
	CHECK(!noRtc.finish_event);

	ArduconRTCGatePlan_t waiting = arduconPlanRTCGate(start - 6, start, finish, 1, 5, 1, 1, 0);
	CHECK(!waiting.pre_power_radio);
	CHECK(!waiting.start_event);
	CHECK(!waiting.finish_event);

	ArduconRTCGatePlan_t prePower = arduconPlanRTCGate(start - 5, start, finish, 1, 5, 1, 1, 0);
	CHECK(prePower.pre_power_radio);
	CHECK(!prePower.start_event);
	CHECK(!prePower.finish_event);

	ArduconRTCGatePlan_t startNow = arduconPlanRTCGate(start, start, finish, 1, 5, 1, 1, 0);
	CHECK(startNow.pre_power_radio);
	CHECK(startNow.start_event);
	CHECK(!startNow.finish_event);

	ArduconRTCGatePlan_t thermalHold = arduconPlanRTCGate(start, start, finish, 1, 5, 1, 1, 1);
	CHECK(!thermalHold.pre_power_radio);
	CHECK(!thermalHold.start_event);
	CHECK(!thermalHold.finish_event);

	ArduconRTCGatePlan_t running = arduconPlanRTCGate(finish - 1, start, finish, 1, 5, 0, 1, 0);
	CHECK(!running.pre_power_radio);
	CHECK(!running.start_event);
	CHECK(!running.finish_event);

	ArduconRTCGatePlan_t finishNow = arduconPlanRTCGate(finish, start, finish, 1, 5, 0, 1, 0);
	CHECK(!finishNow.pre_power_radio);
	CHECK(!finishNow.start_event);
	CHECK(finishNow.finish_event);
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

void checkFoxTimingPlans()
{
	ArduconFoxTimingPlan_t beacon = arduconPlanFoxTiming(ARDUCON_FOX_BEACON, 0);
	CHECK(beacon.on_air_interval_seconds == 600);
	CHECK(beacon.cycle_period_seconds == 600);
	CHECK(beacon.number_of_foxes == 1);
	CHECK(beacon.fox_id_offset == 0);
	CHECK(beacon.pattern_codespeed == 8);
	CHECK(beacon.id_interval_seconds == 600);
	CHECK(!beacon.use_ptt_periodic_reset);

	ArduconFoxTimingPlan_t beaconPeriodicReset = arduconPlanFoxTiming(ARDUCON_FOX_BEACON, 1);
	CHECK(beaconPeriodicReset.on_air_interval_seconds == 60);
	CHECK(beaconPeriodicReset.cycle_period_seconds == 60);
	CHECK(beaconPeriodicReset.id_interval_seconds == 60);
	CHECK(beaconPeriodicReset.use_ptt_periodic_reset);

	ArduconFoxTimingPlan_t spectator = arduconPlanFoxTiming(7, 0);
	CHECK(spectator.on_air_interval_seconds == 600);
	CHECK(spectator.number_of_foxes == 1);

	ArduconFoxTimingPlan_t classic = arduconPlanFoxTiming(ARDUCON_FOX_CLASSIC_5, 1);
	CHECK(classic.on_air_interval_seconds == 60);
	CHECK(classic.cycle_period_seconds == 300);
	CHECK(classic.number_of_foxes == 5);
	CHECK(classic.fox_id_offset == 0);
	CHECK(classic.pattern_codespeed == 8);
	CHECK(classic.id_interval_seconds == 300);
	CHECK(!classic.use_ptt_periodic_reset);

	ArduconFoxTimingPlan_t sprintSlow = arduconPlanFoxTiming(ARDUCON_FOX_SPRINT_S1, 0);
	CHECK(sprintSlow.on_air_interval_seconds == 12);
	CHECK(sprintSlow.cycle_period_seconds == 60);
	CHECK(sprintSlow.number_of_foxes == 5);
	CHECK(sprintSlow.fox_id_offset == ARDUCON_FOX_SPRINT_S1 - 1);
	CHECK(sprintSlow.pattern_codespeed == 8);
	CHECK(sprintSlow.id_interval_seconds == 600);

	ArduconFoxTimingPlan_t sprintFast = arduconPlanFoxTiming(ARDUCON_FOX_SPRINT_F5, 0);
	CHECK(sprintFast.on_air_interval_seconds == 12);
	CHECK(sprintFast.cycle_period_seconds == 60);
	CHECK(sprintFast.number_of_foxes == 5);
	CHECK(sprintFast.fox_id_offset == ARDUCON_FOX_SPRINT_F1 - 1);
	CHECK(sprintFast.pattern_codespeed == 15);
	CHECK(sprintFast.id_interval_seconds == 600);

	ArduconFoxTimingPlan_t reportBattery = arduconPlanFoxTiming(ARDUCON_FOX_REPORT_BATTERY, 0);
	CHECK(reportBattery.on_air_interval_seconds == 30);
	CHECK(reportBattery.cycle_period_seconds == 60);
	CHECK(reportBattery.number_of_foxes == 2);
	CHECK(reportBattery.fox_id_offset == ARDUCON_FOX_REPORT_BATTERY - 1);
	CHECK(reportBattery.pattern_codespeed == 8);
	CHECK(reportBattery.id_interval_seconds == 60);
}

void checkScheduledEventPlans()
{
	const time_t start = minimumEpoch + 200;

	ArduconScheduledEventPlan_t future = arduconPlanScheduledEvent(start - 301, start, start + 1000, 1, 300, 60, 5, 300);
	CHECK(future.seconds_since_sync == 0);
	CHECK(future.fox_counter == 1);
	CHECK(future.transmissions_disabled);
	CHECK(future.use_rtc_for_startstop);
	CHECK(!future.start_active_event_now);
	CHECK(!future.event_in_progress);
	CHECK(future.power_radio_off_until_start);

	ArduconScheduledEventPlan_t nearFuture = arduconPlanScheduledEvent(start - 300, start, start + 1000, 1, 300, 60, 5, 300);
	CHECK(nearFuture.transmissions_disabled);
	CHECK(!nearFuture.power_radio_off_until_start);

	ArduconScheduledEventPlan_t exactStart = arduconPlanScheduledEvent(start, start, start + 1000, 1, 300, 60, 5, 300);
	CHECK(exactStart.seconds_since_sync == 0);
	CHECK(exactStart.fox_counter == 1);
	CHECK(exactStart.transmissions_disabled);
	CHECK(!exactStart.start_active_event_now);
	CHECK(!exactStart.event_in_progress);

	ArduconScheduledEventPlan_t inProgress = arduconPlanScheduledEvent(start + 125, start, start + 1000, 1, 300, 60, 5, 300);
	CHECK(inProgress.seconds_since_sync == 125);
	CHECK(inProgress.fox_counter == 3);
	CHECK(!inProgress.transmissions_disabled);
	CHECK(inProgress.use_rtc_for_startstop);
	CHECK(inProgress.start_active_event_now);
	CHECK(inProgress.event_in_progress);
	CHECK(!inProgress.power_radio_off_until_start);

	ArduconScheduledEventPlan_t nextCycle = arduconPlanScheduledEvent(start + 300, start, start + 1000, 1, 300, 60, 5, 300);
	CHECK(nextCycle.seconds_since_sync == 300);
	CHECK(nextCycle.fox_counter == 1);
	CHECK(!nextCycle.transmissions_disabled);
}

void checkMultiDayScheduleWindows()
{
	const time_t start = minimumEpoch + 3600;
	const time_t finish = start + 7200;
	time_t effectiveStart = 0;
	time_t effectiveFinish = 0;

	CHECK(arduconCurrentOrNextScheduleWindow(start - 60, start, finish, 3, &effectiveStart, &effectiveFinish));
	CHECK(effectiveStart == start);
	CHECK(effectiveFinish == finish);

	CHECK(arduconCurrentOrNextScheduleWindow(finish + 60, start, finish, 3, &effectiveStart, &effectiveFinish));
	CHECK(effectiveStart == start + 86400);
	CHECK(effectiveFinish == finish + 86400);

	CHECK(!arduconCurrentOrNextScheduleWindow(finish + (3 * 86400), start, finish, 3, &effectiveStart, &effectiveFinish));

	ArduconRTCGatePlan_t nextDayStart = arduconPlanRTCGate(start + 86400, start, finish, 3, 5, 1, 1, 0);
	CHECK(nextDayStart.start_event);
	CHECK(nextDayStart.seconds_since_sync == 0);

	ArduconRTCGatePlan_t finishFirstDay = arduconPlanRTCGate(finish, start, finish, 3, 5, 0, 1, 0);
	CHECK(finishFirstDay.finish_event);

	ArduconScheduledEventPlan_t secondDayInProgress = arduconPlanScheduledEvent(start + 86400 + 125, start, finish, 3, 300, 60, 5, 300);
	CHECK(secondDayInProgress.event_in_progress);
	CHECK(secondDayInProgress.seconds_since_sync == 125);
	CHECK(secondDayInProgress.fox_counter == 3);
}

void checkStartStopActions()
{
	CHECK(arduconStartActionForSchedule(ARDUCON_EVENT_SOURCE_POWER_UP, ARDUCON_SCHEDULE_CONFIGURATION_ERROR) == ARDUCON_EVENT_ACTION_START_NOTHING);
	CHECK(arduconStartActionForSchedule(ARDUCON_EVENT_SOURCE_POWER_UP, ARDUCON_SCHEDULE_WAITING_FOR_START) == ARDUCON_EVENT_ACTION_START_WITH_SCHEDULE);
	CHECK(arduconStartActionForSchedule(ARDUCON_EVENT_SOURCE_POWER_UP, ARDUCON_SCHEDULE_EVENT_IN_PROGRESS) == ARDUCON_EVENT_ACTION_START_WITH_SCHEDULE);

	CHECK(arduconStartActionForSchedule(ARDUCON_EVENT_SOURCE_PROGRAMMATIC, ARDUCON_SCHEDULE_CONFIGURATION_ERROR) == ARDUCON_EVENT_ACTION_START_EVENT_NOW);
	CHECK(arduconStartActionForSchedule(ARDUCON_EVENT_SOURCE_PROGRAMMATIC, ARDUCON_SCHEDULE_WAITING_FOR_START) == ARDUCON_EVENT_ACTION_START_EVENT_NOW);
	CHECK(arduconStartActionForSchedule(ARDUCON_EVENT_SOURCE_PROGRAMMATIC, ARDUCON_SCHEDULE_DID_NOT_START) == ARDUCON_EVENT_ACTION_START_EVENT_NOW);
	CHECK(arduconStartActionForSchedule(ARDUCON_EVENT_SOURCE_PROGRAMMATIC, ARDUCON_SCHEDULE_WILL_NEVER_RUN) == ARDUCON_EVENT_ACTION_START_EVENT_NOW);
	CHECK(arduconStartActionForSchedule(ARDUCON_EVENT_SOURCE_PROGRAMMATIC, ARDUCON_SCHEDULE_EVENT_IN_PROGRESS) == ARDUCON_EVENT_ACTION_START_WITH_SCHEDULE);

	CHECK(arduconStartActionForSchedule(ARDUCON_EVENT_SOURCE_PUSHBUTTON, ARDUCON_SCHEDULE_CONFIGURATION_ERROR) == ARDUCON_EVENT_ACTION_START_EVENT_NOW);
	CHECK(arduconStartActionForSchedule(ARDUCON_EVENT_SOURCE_PUSHBUTTON, ARDUCON_SCHEDULE_WAITING_FOR_START) == ARDUCON_EVENT_ACTION_START_TRANSMISSIONS_NOW);
	CHECK(arduconStartActionForSchedule(ARDUCON_EVENT_SOURCE_PUSHBUTTON, ARDUCON_SCHEDULE_WILL_NEVER_RUN) == ARDUCON_EVENT_ACTION_START_WITH_SCHEDULE);
	CHECK(arduconStartActionForSchedule(ARDUCON_EVENT_SOURCE_PUSHBUTTON, ARDUCON_SCHEDULE_EVENT_IN_PROGRESS) == ARDUCON_EVENT_ACTION_START_WITH_SCHEDULE);

	CHECK(arduconStopActionForSchedule(ARDUCON_EVENT_SOURCE_PROGRAMMATIC, ARDUCON_SCHEDULE_EVENT_IN_PROGRESS) == ARDUCON_EVENT_ACTION_START_NOTHING);
	CHECK(arduconStopActionForSchedule(ARDUCON_EVENT_SOURCE_PUSHBUTTON, ARDUCON_SCHEDULE_WAITING_FOR_START) == ARDUCON_EVENT_ACTION_START_TRANSMISSIONS_NOW);
	CHECK(arduconStopActionForSchedule(ARDUCON_EVENT_SOURCE_PUSHBUTTON, ARDUCON_SCHEDULE_CONFIGURATION_ERROR) == ARDUCON_EVENT_ACTION_START_NOTHING);
	CHECK(arduconStopActionForSchedule(ARDUCON_EVENT_SOURCE_PUSHBUTTON, ARDUCON_SCHEDULE_EVENT_IN_PROGRESS) == ARDUCON_EVENT_ACTION_START_NOTHING);
}
}

int main()
{
	checkScheduleClassification();
	checkStartAndFinishBoundaries();
	checkRadioPrePower();
	checkRTCGatePlans();
	checkFoxSlotLogic();
	checkFoxTimingPlans();
	checkScheduledEventPlans();
	checkMultiDayScheduleWindows();
	checkStartStopActions();

	std::cout << "Scheduler logic unit tests passed." << '\n';
	return 0;
}
