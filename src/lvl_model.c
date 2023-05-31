#include "lvl_model.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lvl_model,LOG_LEVEL_DBG);





/// @brief helper to create status msg derived from dimmable_ctx
/// @param dimmable the struct that is used for storing in this file
/// @param status a status instance the data should be saved to
static void create_dimmable_status(const struct dimmable_ctx * d_ctx, 
			struct bt_mesh_lvl_status * status)
{
	status->remaining_time = d_ctx->remaining_time ? d_ctx->remaining_time : 
		k_ticks_to_ms_ceil32(k_work_delayable_remaining_get(&d_ctx->work));
		status->target = mesh_level2struct_level(d_ctx->target_lvl);
		status->current = mesh_level2struct_level(d_ctx->current_lvl);
	LOG_DBG("created STATUS from dimmable_ctx. rem: %d, cur: %d, tar: %d", 
		status->remaining_time, d_ctx->current_lvl, d_ctx->target_lvl);
}





/// @brief schedules a new SET level transition
/// @param set the set struct that contains the target level
/// @param ctx the dimmable stuct instance that should be updated
static void dimmable_transition_start(const struct bt_mesh_lvl_set *set, 
			struct dimmable_ctx *ctx)
{
	/** calculate needed parameters */
	//how many steps (of size PWM_SIZE_STEP) are needed to reach target value
	uint32_t step_cnt = abs(set->lvl - ctx->current_lvl) / PWM_SIZE_STEP;		
	uint32_t time = set->transition ? set->transition->time : 0;				
	uint32_t delay = set->transition ? set->transition->delay : 0;				

	//save parameters in the helper struct dimmable_ctx
	ctx->target_lvl = struct_level2mesh_level(set->lvl);
	ctx->time_period = (step_cnt ? time / step_cnt : 0);
	ctx->remaining_time = time;

	//to remove app bug that does not update state, also set current_lvl 
	//to target_lvl if time & delay is 0
	//exp: this is because the app does show the current_lvl 
	//exp: instead of the target level when delay & time is 0
	if(!time && !delay)
		ctx->current_lvl = ctx->target_lvl;

	//work will be started after delay time
	k_work_reschedule(&ctx->work, K_MSEC(delay));	
	LOG_INF("SET-trans started. Delay: %d, Trans: %d, t/step: %d, start-lvl: %d, target-lvl: %d ", 
		delay, time, ctx->time_period , ctx->current_lvl, ctx->target_lvl);
}





/// @brief Schedules a continuous change of level (MOVE)
/// @param set the move_set struct that contains moveStepSize and pauseTime/step
/// @param ctx the dimmable stuct instance that should be updated
static void dimmable_move_start(const struct bt_mesh_lvl_move_set *set, 
			struct dimmable_ctx *ctx)
{	
	ctx->move_step = set->delta;
	//check if to start or stop delta
	if(0 == ctx->move_step) {
		//stop the work item!
		k_work_cancel_delayable(&ctx->work);
		ctx->target_lvl = ctx->current_lvl;
		ctx->remaining_time = 0;
		LOG_INF("MOVE-trans STOPPED. lvl: %d ", ctx->current_lvl);
		return;
	}	//exp if delta is not 0
	else if(ctx->move_step > 0) {
		ctx->target_lvl = UINT16_MAX;
	} else {	//ctx->move_step < 0
		ctx->target_lvl = 0;
	}
	uint32_t step_cnt = 
		abs((int32_t) ctx->target_lvl - ctx->current_lvl) / PWM_SIZE_STEP;
	uint32_t delay = set->transition ? set->transition->delay : 0;
	/*for delta, not the transition form *set is used, 
	but the theoretical one is calculated 
	how long it would take to reach max or min level*/
	uint32_t time = (step_cnt * set->transition->time * PWM_SIZE_STEP) 
		/ abs(ctx->move_step);
		//in ms!

	//save parameters in the helper struct dimmable_ctx
	ctx->time_period = (step_cnt ? time / step_cnt : 0);
	ctx->remaining_time = time;

	k_work_reschedule(&ctx->work, K_MSEC(delay));
	LOG_INF("MOVE-trans started. Delay: %d, Trans: %d, t/step: %d, #steps: %d, start-lvl: %d, move-step: %d ", 
		delay, 
		time, 
		ctx->time_period, 
		step_cnt, 
		ctx->current_lvl, 
		ctx->move_step);
}





void dimmable_set(struct bt_mesh_lvl_srv *srv, 
			struct bt_mesh_msg_ctx *ctx,
		    const struct bt_mesh_lvl_set *set,
		    struct bt_mesh_lvl_status *rsp)
{
	struct dimmable_ctx * d_ctx = CONTAINER_OF(srv, struct dimmable_ctx, srv);
	dimmable_transition_start(set, d_ctx);

	if(rsp) {
		create_dimmable_status(d_ctx, rsp);
		LOG_DBG("Sent status after SET");
	}
}





void dimmable_get(struct bt_mesh_lvl_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    struct bt_mesh_lvl_status *rsp)
{
	struct dimmable_ctx * dimmable = 
		CONTAINER_OF(srv, struct dimmable_ctx, srv);
	create_dimmable_status(dimmable, rsp);
}





void dimmable_move_set(struct bt_mesh_lvl_srv *srv,
			struct bt_mesh_msg_ctx *ctx,
			const struct bt_mesh_lvl_move_set *move_set,
			struct bt_mesh_lvl_status *rsp)
{
	struct dimmable_ctx * d_ctx = CONTAINER_OF(srv, struct dimmable_ctx, srv);
	dimmable_move_start(move_set, d_ctx);
	if(rsp) {
		create_dimmable_status(d_ctx, rsp);
		LOG_DBG("Sent status after MOVE");
	}
}





void dimmable_work(struct k_work * work)
{
	struct dimmable_ctx *d_ctx = 
		CONTAINER_OF(work, struct dimmable_ctx, work.work);
	//subtract one time period as one step will be performed now
	d_ctx->remaining_time -= d_ctx->time_period;	

	// * check, if transition can be regarded complete
	//this is the case if remaining time is shorter than one time step 
	//or when difference between current & target is smaller than PWM_SIZE_STEP
	if((d_ctx->remaining_time <= d_ctx->time_period) || 
		(abs(d_ctx->target_lvl - d_ctx->current_lvl) <= PWM_SIZE_STEP))
	{
		//update struct
		d_ctx->current_lvl = d_ctx->target_lvl;
		d_ctx->remaining_time = 0;
		//create appropriate status message & publish
		struct bt_mesh_lvl_status tempStatus;
		create_dimmable_status(d_ctx, &tempStatus);
		bt_mesh_lvl_srv_pub(&d_ctx->srv, NULL, &tempStatus);
		LOG_DBG("srv PUB, NO reschedule");
		LOG_INF("Trans work COMPLETED.");
	} else {	
	// * transition not yet complete
		if (d_ctx->target_lvl > d_ctx->current_lvl) {
			//if target value is higher than current value, 
			//increase current value by one step
			d_ctx->current_lvl += PWM_SIZE_STEP;
		} else {
			//if target value is lower than current value, 
			//decrease current value by one step
			d_ctx->current_lvl -= PWM_SIZE_STEP;
		}
		//reschedule work to be executed again after one time period
		k_work_reschedule(&d_ctx->work, K_MSEC(d_ctx->time_period));
	}
	//set led to new value
	d_ctx->pwm_output(d_ctx->current_lvl);
	LOG_DBG("Cur-lvl set to: %u/65535", d_ctx->current_lvl);
}