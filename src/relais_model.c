#include "relais_model.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(relais_model,LOG_LEVEL_DBG);





/// @brief get the current status (including transition time) and save it in the
/// status parameter
/// @param relais the relais that should be queried
/// @param status here the current status will be saved
static void relais_status(const struct relais_ctx *relais, 
			struct bt_mesh_onoff_status *status)
{
	/* Do not include delay in the remaining time. */
	status->remaining_time = relais->remaining ? relais->remaining :	
		k_ticks_to_ms_ceil32(k_work_delayable_remaining_get(&relais->work));	
	/**
	 * in case relais->remaining is ZERO 
	 * (which can happen when relais_transition_start was executed), 
	 * it is checked for when this execution is finished
	 */
	status->target_on_off = relais->value;
	/* As long as the transition is in progress, the onoff state is "on": */
	status->present_on_off = relais->value || status->remaining_time;
	LOG_DBG("created STATUS from relais_ctx. rem: %d, pres: %d, fut: %d", 
		status->remaining_time, 
		status->present_on_off, 
		status->target_on_off);
}





/// @brief schedules the relais toggle
/// @param relais the relais that should be toggled
static void relais_transition_start(struct relais_ctx *relais)
{
	//exp As long as the transition is in progress, 
	//exp the onoff state shall be "on"
	relais->relais_output(true);
	//the work will be scheduled after the "remaining"-value
	k_work_reschedule(&relais->work, K_MSEC(relais->remaining));	
	relais->remaining = 0;		//can be already set to 0.
	/** if status is asked and this is 0, ongoing work is checked anyway.
	 * If work is ongoing, the time until it is finished is returned 
	 * as remaining time
	*/
LOG_DBG("SET-trans started.");
}





/// @brief set a (future) status for the relais
/// @param srv server instance
/// @param ctx context information for received message
/// @param set new state that should be set
/// @param rsp if provided, status response is saved here
void relais_set(struct bt_mesh_onoff_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    const struct bt_mesh_onoff_set *set,
		    struct bt_mesh_onoff_status *rsp)
{
	struct relais_ctx *relais = CONTAINER_OF(srv, struct relais_ctx, srv);

	//when future value == current value
	if (set->on_off == relais->value) {
		goto respond;
	}

	relais->value = set->on_off;
	if (!bt_mesh_model_transition_time(set->transition)) {
		//execute instantly if the transition time is 0
		relais->remaining = 0;
		relais->relais_output(set->on_off);
		goto respond;
	}

	relais->remaining = set->transition->time;

	if (set->transition->delay) {
		k_work_reschedule(&relais->work, K_MSEC(set->transition->delay));
	} else {
		relais_transition_start(relais);
	}

respond:
	if (rsp) {	//only send response if a status pointer was given
		relais_status(relais, rsp);
	}
	LOG_INF("SET executed: Delay %d, Trans: %d, pres: %d, fut: %d", 
		set->transition->delay, 
		set->transition->time, 
		relais->value, 
		set->on_off);
}





/// @brief get the status of the relais
/// @param srv server instance
/// @param ctx context information for received message
/// @param rsp if provided, status response is saved here
void relais_get(struct bt_mesh_onoff_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    struct bt_mesh_onoff_status *rsp)
{
	struct relais_ctx *relais = CONTAINER_OF(srv, struct relais_ctx, srv);
	relais_status(relais, rsp);
}





void relais_work(struct k_work *work)
{
	struct relais_ctx *relais = CONTAINER_OF(
		work, struct relais_ctx, work.work);

	if (relais->remaining) {
		relais_transition_start(relais);
	} else {
		//set appliance to target value
		relais->relais_output(relais->value);

		/* Publish the new value at the end of the transition */
		struct bt_mesh_onoff_status tempStatus;
		relais_status(relais, &tempStatus);
		bt_mesh_onoff_srv_pub(&relais->srv, NULL, &tempStatus);
		LOG_DBG("srv PUB");
		LOG_INF("Trans work COMPLETED");
	}
}