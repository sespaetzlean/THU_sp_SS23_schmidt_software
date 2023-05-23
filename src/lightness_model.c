#include "lightness_model.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lightness_model,LOG_LEVEL_DBG);


/// @brief 
/// @param set 
/// @param ctx 
static void light_transition_start(const struct bt_mesh_lightness_set *set, struct lightness_ctx *l_ctx)
{
	//calculate parameters needed for state transition
	uint32_t step_cnt = abs(set->lvl - l_ctx->current_lvl) / PWM_SIZE_STEP;		//calculate the number of steps needed to reach the target level
	uint32_t time = set->transition ? set->transition->time : 0;
	uint32_t delay = set->transition ? set->transition->delay : 0;

	//save parameters in the helper struct lightness_ctx
	l_ctx->target_lvl = set->lvl;
	l_ctx->time_per = (step_cnt ? time / step_cnt : 0);
	l_ctx->rem_time = time;

	//TODO: check for this bug
	//to remove app bug that does not update state, also set current_lvl to target_lvl if time & delay is 0
	//exp: this is because the app does show the current_lvl instead of the target level when delay & time is 0
	// if(!time && !delay)
	// 	l_ctx->current_lvl = l_ctx->target_lvl;

	k_work_reschedule(&l_ctx->work, K_MSEC(delay));	//work will be started after delay time
	LOG_INF("Transition started. Delay: %d, Transition: %d, Start level(ctx): %d, target level(ctx): %d ",delay, time, l_ctx->current_lvl, l_ctx->target_lvl);
}





/// @brief get the current status and save it in the status parameter
/// @param l_ctx helper lightness struct
/// @param status a status instance the data should be saved to
static void lightness_status(const struct lightness_ctx * l_ctx, struct bt_mesh_lightness_status * status)
{
	status->remaining_time = l_ctx->remaining_time ? l_ctx->remaining_time : 
		k_ticks_to_ms_ceil32(k_work_delayable_remaining_get(&l_ctx->work));
	status->target = l_ctx->target_lvl;
	status->current = l_ctx->current_lvl;
	LOG_DBG("Created a status message: time %d, status_target %d, status_current %d", status->remaining_time, status->target, status->current);
}





void light_set(struct bt_mesh_lightness_srv *srv,
		      struct bt_mesh_msg_ctx *ctx,
		      const struct bt_mesh_lightness_set *set,
		      struct bt_mesh_lightness_status *rsp)
{
	struct lightness_ctx *l_ctx = CONTAINER_OF(srv, struct lightness_ctx, srv);
	light_transition_start(set, l_ctx);

	if(rsp)
	{
		lightness_status(l_ctx, rsp);
		LOG_DBG("Sent a status update after set");
	}
}





void dimmable_get(struct bt_mesh_lightness_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    struct bt_mesh_lightness_status *rsp)
{
	struct lightness_ctx * l_ctx = CONTAINER_OF(srv, struct lightness_ctx, srv);
	lightness_status(l_ctx, rsp);
}















void lightness_work(struct k_work *work)
{
	struct lightness_ctx *l_ctx = CONTAINER_OF(work, struct lightness_ctx, work.work);
	l_ctx->remaining_time -= l_ctx->time_period;	//subtract one time period as one step will be performed now

	//check, if transition can be regarded complete
	//this is the case if remaining time is shorter than one time step 
	//or when the difference between curren t& target is smaller than one PWM_SIZE_STEP
	if((l_ctx->remaining_time <= l_ctx->time_period) || (abs(l_ctx->target_lvl - l_ctx->current_lvl) <= PWM_SIZE_STEP))
	{
		//update struct
		l_ctx->current_lvl = l_ctx->target_lvl;
		l_ctx->remaining_time = 0;
		//create appropriate status message
		struct bt_mesh_lvl_status status;
		dimmable_status(l_ctx, &status);
		//and publish the message
		bt_mesh_lvl_srv_pub(&l_ctx->srv, NULL, &status);
		LOG_DBG("Transition completed");
	} else {	//transition not yet complete
		if (l_ctx->target_lvl > l_ctx->current_lvl) {
			//if target value is higher than current value, increase current value by one step
			l_ctx->current_lvl += PWM_SIZE_STEP;
		} else {
			//if target value is lower than current value, decrease current value by one step
			l_ctx->current_lvl -= PWM_SIZE_STEP;
		}
		//reschedule work to be executed again after one time period
		k_work_reschedule(&l_ctx->work, K_MSEC(l_ctx->time_period));
	}
	//set led to new value
	l_ctx->pwm_output(l_ctx->current_lvl);
	//log information
	LOG_DBG("Current light lvl set to: %u/65535\n", l_ctx->current_lvl);
}