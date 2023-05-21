#include "lvl_model.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(lvl_model,LOG_LEVEL_DBG);




/// @brief schedules a new level transition
/// @param dimmable the dimmable stuct instance that should be updated
static void dimmable_transition_start(struct dimmable_ctx * dimmable)
{
	LOG_INF("transition in progress. Start level: %d, target level: %d ", dimmable->value, dimmable->target_value);
	//TODO: smooth transition to program, should reschedul it self all the time until target level is reached
	k_work_reschedule(&dimmable->work, K_MSEC(dimmable->remaining));	//the work will be scheduled again after the "remaining"-value
	dimmable->remaining = 0;		//so remaining can be set to 0 now
}




/// @brief get the current status and save it in the status parameter
/// @param dimmable the struct that is used for storing in this file
/// @param status a status instance the data should be saved to
static void dimmable_status(const struct dimmable_ctx * dimmable, struct bt_mesh_lvl_status * status)
{
	status->remaining_time = dimmable->remaining ? dimmable->remaining : 
		k_ticks_to_ms_ceil32(k_work_delayable_remaining_get(&dimmable->work));
		status->target = dimmable->target_value;
		status->current = dimmable->value;
}





void dimmable_set(struct bt_mesh_lvl_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    const struct bt_mesh_lvl_set *set,
		    struct bt_mesh_lvl_status *rsp)
{
	struct dimmable_ctx * dimmable = CONTAINER_OF(srv, struct dimmable_ctx, srv);

	//if future value == current value
	if(set->lvl == dimmable->target_value) {
		goto respond;
	}

	dimmable->target_value = set->lvl;
	if (!bt_mesh_model_transition_time(set->transition)) {
		//execute instantly if the transition time is set to 0
		dimmable->remaining = 0;
		LOG_INF("Target level set immediately to %d", dimmable->target_value);
		//consequently, current value can be set to target value directly as well
		dimmable->value = dimmable->target_value;
		goto respond;
	}
	
	dimmable->remaining = set->transition->time;

	if (set->transition->delay) {
		LOG_DBG("New updated will be set delayed by: %d", set->transition->delay);
		k_work_reschedule(&dimmable->work, K_MSEC(set->transition->delay));
	} else {
		dimmable_transition_start(dimmable);
	}

respond:
	if(rsp) {
		dimmable_status(dimmable, rsp);
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
	struct dimmable_ctx * dimmable = CONTAINER_OF(work, struct dimmable_ctx, work.work);

	if(dimmable->remaining)
	{
		dimmable_transition_start(dimmable);
	} else {
		LOG_INF("New level set via work instantly (due to delay??? (Vermutung)) to %d ", dimmable->target_value);
		//current value is consequently directly set to target value as well, save like this in struct
		dimmable->value = dimmable->target_value;
		//TODO: set led to new value

		//Publish
		struct bt_mesh_lvl_status status;
		dimmable_status(dimmable, &status);
		bt_mesh_lvl_srv_pub(&dimmable->srv, NULL, &status);
	}
}