/*
 *  MIT License
 *
 *  Copyright (c) 2021 DigitalConfections
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#ifndef SCHEDULER_LOGIC_H_
#define SCHEDULER_LOGIC_H_

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	ARDUCON_SCHEDULE_WAITING_FOR_START,
	ARDUCON_SCHEDULE_CONFIGURATION_ERROR,
	ARDUCON_SCHEDULE_DID_NOT_START,
	ARDUCON_SCHEDULE_WILL_NEVER_RUN,
	ARDUCON_SCHEDULE_EVENT_IN_PROGRESS
} ArduconScheduleState_t;

typedef enum
{
	ARDUCON_EVENT_SOURCE_POWER_UP,
	ARDUCON_EVENT_SOURCE_PUSHBUTTON,
	ARDUCON_EVENT_SOURCE_PROGRAMMATIC
} ArduconEventActionSource_t;

typedef enum
{
	ARDUCON_EVENT_ACTION_START_NOTHING,
	ARDUCON_EVENT_ACTION_START_EVENT_NOW,
	ARDUCON_EVENT_ACTION_START_TRANSMISSIONS_NOW,
	ARDUCON_EVENT_ACTION_START_WITH_SCHEDULE
} ArduconEventAction_t;

typedef struct
{
	time_t current_epoch;
	time_t start_epoch;
	time_t finish_epoch;
	time_t minimum_epoch;
	uint8_t use_rtc_for_startstop;
	uint8_t transmissions_disabled;
} ArduconScheduleConfig_t;

typedef struct
{
	int seconds_since_sync;
	int fox_counter;
	uint8_t transmissions_disabled;
	uint8_t use_rtc_for_startstop;
	uint8_t start_active_event_now;
	uint8_t event_in_progress;
	uint8_t power_radio_off_until_start;
} ArduconScheduledEventPlan_t;

typedef struct
{
	uint8_t pre_power_radio;
	uint8_t start_event;
	uint8_t finish_event;
} ArduconRTCGatePlan_t;

static inline int arduconSchedulerClamp(int low, int value, int high)
{
	if(value < low)
	{
		return low;
	}
	if(value > high)
	{
		return high;
	}
	return value;
}

static inline ArduconScheduleState_t arduconClassifySchedule(const ArduconScheduleConfig_t* config)
{
	if(!config)
	{
		return ARDUCON_SCHEDULE_CONFIGURATION_ERROR;
	}

	if((config->finish_epoch < config->minimum_epoch) || (config->start_epoch < config->minimum_epoch) || (config->current_epoch < config->minimum_epoch))
	{
		return ARDUCON_SCHEDULE_CONFIGURATION_ERROR;
	}

	if(config->finish_epoch <= config->start_epoch)
	{
		return ARDUCON_SCHEDULE_CONFIGURATION_ERROR;
	}

	if(config->current_epoch > config->finish_epoch)
	{
		return ARDUCON_SCHEDULE_CONFIGURATION_ERROR;
	}

	if(config->current_epoch > config->start_epoch)
	{
		if(config->transmissions_disabled)
		{
			return ARDUCON_SCHEDULE_DID_NOT_START;
		}
		return ARDUCON_SCHEDULE_EVENT_IN_PROGRESS;
	}

	if(!config->use_rtc_for_startstop)
	{
		return ARDUCON_SCHEDULE_WILL_NEVER_RUN;
	}

	return ARDUCON_SCHEDULE_WAITING_FOR_START;
}

static inline uint8_t arduconShouldPrePowerRadio(time_t current_epoch, time_t start_epoch, int lead_seconds, uint8_t thermal_shutdown)
{
	return ((current_epoch >= (start_epoch - lead_seconds)) && !thermal_shutdown);
}

static inline uint8_t arduconShouldStartScheduledEvent(time_t current_epoch, time_t start_epoch, time_t finish_epoch, uint8_t thermal_shutdown)
{
	return ((current_epoch >= start_epoch) && (current_epoch < finish_epoch) && !thermal_shutdown);
}

static inline uint8_t arduconShouldFinishScheduledEvent(time_t current_epoch, time_t finish_epoch)
{
	return (current_epoch >= finish_epoch);
}

static inline ArduconRTCGatePlan_t arduconPlanRTCGate(time_t current_epoch, time_t start_epoch, time_t finish_epoch, int pre_power_lead_seconds, uint8_t transmissions_disabled, uint8_t use_rtc_for_startstop, uint8_t thermal_shutdown)
{
	ArduconRTCGatePlan_t plan;
	plan.pre_power_radio = 0;
	plan.start_event = 0;
	plan.finish_event = 0;

	if(!use_rtc_for_startstop)
	{
		return plan;
	}

	if(transmissions_disabled)
	{
		plan.pre_power_radio = arduconShouldPrePowerRadio(current_epoch, start_epoch, pre_power_lead_seconds, thermal_shutdown);
		plan.start_event = arduconShouldStartScheduledEvent(current_epoch, start_epoch, finish_epoch, thermal_shutdown);
	}
	else
	{
		plan.finish_event = arduconShouldFinishScheduledEvent(current_epoch, finish_epoch);
	}

	return plan;
}

static inline uint8_t arduconCurrentFoxShouldTransmit(int number_of_foxes, int fox, int fox_counter, int fox_id_offset)
{
	return ((number_of_foxes == 1) || ((number_of_foxes > 1) && (fox == (fox_counter + fox_id_offset))));
}

static inline int arduconScheduledFoxCounter(int seconds_since_sync, int cycle_period_seconds, int on_air_interval_seconds, int number_of_foxes)
{
	if((cycle_period_seconds <= 0) || (on_air_interval_seconds <= 0) || (number_of_foxes <= 0))
	{
		return 1;
	}

	return arduconSchedulerClamp(1, 1 + ((seconds_since_sync % cycle_period_seconds) / on_air_interval_seconds), number_of_foxes);
}

static inline ArduconScheduledEventPlan_t arduconPlanScheduledEvent(time_t current_epoch, time_t start_epoch, int cycle_period_seconds, int on_air_interval_seconds, int number_of_foxes, int radio_power_off_threshold_seconds)
{
	ArduconScheduledEventPlan_t plan;
	plan.seconds_since_sync = 0;
	plan.fox_counter = 1;
	plan.transmissions_disabled = 1;
	plan.use_rtc_for_startstop = 1;
	plan.start_active_event_now = 0;
	plan.event_in_progress = 0;
	plan.power_radio_off_until_start = 0;

	if(start_epoch < current_epoch)
	{
		plan.seconds_since_sync = (int)(current_epoch - start_epoch);
		plan.fox_counter = arduconScheduledFoxCounter(plan.seconds_since_sync, cycle_period_seconds, on_air_interval_seconds, number_of_foxes);
		plan.transmissions_disabled = 0;
		plan.start_active_event_now = 1;
		plan.event_in_progress = 1;
	}
	else
	{
		plan.power_radio_off_until_start = (start_epoch > (current_epoch + radio_power_off_threshold_seconds));
	}

	return plan;
}

static inline ArduconEventAction_t arduconStartActionForSchedule(ArduconEventActionSource_t activation_source, ArduconScheduleState_t schedule_state)
{
	if(activation_source == ARDUCON_EVENT_SOURCE_POWER_UP)
	{
		if(schedule_state == ARDUCON_SCHEDULE_CONFIGURATION_ERROR)
		{
			return ARDUCON_EVENT_ACTION_START_NOTHING;
		}
		return ARDUCON_EVENT_ACTION_START_WITH_SCHEDULE;
	}

	if(activation_source == ARDUCON_EVENT_SOURCE_PROGRAMMATIC)
	{
		if(schedule_state == ARDUCON_SCHEDULE_EVENT_IN_PROGRESS)
		{
			return ARDUCON_EVENT_ACTION_START_WITH_SCHEDULE;
		}
		return ARDUCON_EVENT_ACTION_START_EVENT_NOW;
	}

	if(schedule_state == ARDUCON_SCHEDULE_CONFIGURATION_ERROR)
	{
		return ARDUCON_EVENT_ACTION_START_EVENT_NOW;
	}
	if(schedule_state == ARDUCON_SCHEDULE_WAITING_FOR_START)
	{
		return ARDUCON_EVENT_ACTION_START_TRANSMISSIONS_NOW;
	}

	return ARDUCON_EVENT_ACTION_START_WITH_SCHEDULE;
}

static inline ArduconEventAction_t arduconStopActionForSchedule(ArduconEventActionSource_t activation_source, ArduconScheduleState_t schedule_state)
{
	if(activation_source == ARDUCON_EVENT_SOURCE_PROGRAMMATIC)
	{
		return ARDUCON_EVENT_ACTION_START_NOTHING;
	}

	if(schedule_state == ARDUCON_SCHEDULE_WAITING_FOR_START)
	{
		return ARDUCON_EVENT_ACTION_START_TRANSMISSIONS_NOW;
	}

	return ARDUCON_EVENT_ACTION_START_NOTHING;
}

#ifdef __cplusplus
}
#endif

#endif /* SCHEDULER_LOGIC_H_ */
