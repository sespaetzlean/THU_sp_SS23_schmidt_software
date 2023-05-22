#include "lvl_model.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lvl_model,LOG_LEVEL_DBG);





// ======================================== helper functions =================================//

/// @brief convert a signed int to an unsigned int 
/// @param inputLevel the signed int
/// @return the mapped unsigned int
static uint16_t input_level2bt_level(int16_t inputLevel)
{
	return (uint16_t) 32768 + (uint16_t) inputLevel;
}

/// @brief convert an unsigned int to a signed int 
/// @param bt_level the unsigned int
/// @return the mapped signed int
static int16_t bt_level2input_level(uint16_t bt_level)
{
	return (int16_t) bt_level - (int16_t) 32768;
}



/// @brief schedules a new level transition
/// @param dimmable the dimmable stuct instance that should be updated
static void dimmable_transition_start(const struct bt_mesh_lvl_set *set, struct dimmable_ctx * ctx)
{
	//calculate needed parameters
	uint32_t step_cnt = abs(set->lvl - ctx->current_lvl) / PWM_SIZE_STEP;		//how many steps (of size PWM_SIZE_STEP) are needed to reach the target value
	uint32_t time = set->transition ? set->transition->time : 0;				//if 0 or NULL, transition time is 0
	uint32_t delay = set->transition ? set->transition->delay : 0;				//if 0 or NULL, delay time is 0

	//save parameters in the helper struct dimmable_ctx
	ctx->target_lvl = set->lvl;
	ctx->time_period = (step_cnt ? time / step_cnt : 0);
	ctx->remaining_time = time;

	//to remove app bug that does not update state, also set current_lvl to target_lvl if time & delay is 0
	//exp: this is because the app does show the current_lvl instead of the target level when delay & time is 0
	if(!time && !delay)
		ctx->current_lvl = ctx->target_lvl;

	k_work_reschedule(&ctx->work, K_MSEC(delay));	//work will be started after delay time
	LOG_INF("Transition started. Delay: %d, Transition: %d, \nStart level: %d, target level: %d ",delay, time, ctx->current_lvl, ctx->target_lvl);
}




/// @brief get the current status and save it in the status parameter
/// @param dimmable the struct that is used for storing in this file
/// @param status a status instance the data should be saved to
static void dimmable_status(const struct dimmable_ctx * d_ctx, struct bt_mesh_lvl_status * status)
{
	status->remaining_time = d_ctx->remaining_time ? d_ctx->remaining_time : 
		k_ticks_to_ms_ceil32(k_work_delayable_remaining_get(&d_ctx->work));
		status->target = d_ctx->target_lvl;
		status->current = d_ctx->current_lvl;
	LOG_DBG("Created a status message. time %d, target %d, current %d", status->remaining_time, status->target, status->current);
}





void dimmable_set(struct bt_mesh_lvl_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    const struct bt_mesh_lvl_set *set,
		    struct bt_mesh_lvl_status *rsp)
{
	struct dimmable_ctx * d_ctx = CONTAINER_OF(srv, struct dimmable_ctx, srv);
	dimmable_transition_start(set, d_ctx);

	if(rsp) {
		dimmable_status(d_ctx, rsp);
		LOG_DBG("Sent a status update after set");
	}
}





void dimmable_get(struct bt_mesh_lvl_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    struct bt_mesh_lvl_status *rsp)
{
	struct dimmable_ctx * dimmable = CONTAINER_OF(srv, struct dimmable_ctx, srv);
	dimmable_status(dimmable, rsp);
}






void dimmable_work(struct k_work * work)
{
	struct dimmable_ctx *d_ctx = CONTAINER_OF(work, struct dimmable_ctx, work.work);
	d_ctx->remaining_time -= d_ctx->time_period;	//subtract one time period as one step will be performed now

	//check, if transition can be regarded complete
	//this is the case if remaining time is shorter than one time step 
	//or when the difference between curren t& target is smaller than one PWM_SIZE_STEP
	if((d_ctx->remaining_time <= d_ctx->time_period) || (abs(d_ctx->target_lvl - d_ctx->current_lvl) <= PWM_SIZE_STEP))
	{
		//update struct
		d_ctx->current_lvl = d_ctx->target_lvl;
		d_ctx->remaining_time = 0;
		//create appropriate status message
		struct bt_mesh_lvl_status status = {
			.current = d_ctx->target_lvl,
			.target = d_ctx->target_lvl,
		};
		//and publish the message
		bt_mesh_lvl_srv_pub(&d_ctx->srv, NULL, &status);
		LOG_DBG("Transition completed");
	} else {	//transition not yet complete
		if (d_ctx->target_lvl > d_ctx->current_lvl) {
			//if target value is higher than current value, increase current value by one step
			d_ctx->current_lvl += PWM_SIZE_STEP;
		} else {
			//if target value is lower than current value, decrease current value by one step
			d_ctx->current_lvl -= PWM_SIZE_STEP;
		}
		//reschedule work to be executed again after one time period
		k_work_reschedule(&d_ctx->work, K_MSEC(d_ctx->time_period));
	}
	//set led to new value
	//TODO
	//lc_pwm_output_set(d_ctx->pwm_specs, d_ctx->current_lvl);
	//log information
	LOG_DBG("Current light lvl set to: %u/65535\n", d_ctx->current_lvl);
}



void dimmable_init(const struct device *pwm_dev[], size_t dev_count)
{
	//TODO
	// for (size_t i = 0; i < dev_count; i++)
	// {
	// 	lc_pwm_output_init(pwm_dev[i]);
	// }
	return;
	
}