#include "relais_model.h"


/// @brief schedules the relais toggle
/// @param relais the relais that should be toggled
static void relais_transition_start(struct relais_ctx *relais)
{
	//exp As long as the transition is in progress, the onoff state shall be "on"
	relais->relais_output(true);
	k_work_reschedule(&relais->work, K_MSEC(relais->remaining));	//the work will be scheduled again after the "remaining"-value
	relais->remaining = 0;		//so remaining can be set to 0 now
}

/// @brief get the current status (including transition time) and save it in the status parameter
/// @param relais the relais that should be queried
/// @param status here the current status will be saved
static void relais_status(const struct relais_ctx *relais, struct bt_mesh_onoff_status *status)
{
	/* Do not include delay in the remaining time. */
	status->remaining_time = relais->remaining ? relais->remaining :	
		k_ticks_to_ms_ceil32(k_work_delayable_remaining_get(&relais->work));	//in case relais->remaining is ZERO (which can happen when relais_transition_start was executed), it is checked for when this execution is scheduled)
	status->target_on_off = relais->value;
	/* As long as the transition is in progress, the onoff state is "on": */
	status->present_on_off = relais->value || status->remaining_time;
}



/// @brief set a (future) status for the relais
/// @param srv server instance	(should include current OnOff-Value)
/// @param ctx context information for received message, (source, destination, ...)
/// @param set new state that should be set
/// @param rsp give in case you want a status response after set
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
		//execute if the transition time is already 0
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
	if (rsp) {	//only send response if a status pointer was given. Use the before declared function
		relais_status(relais, rsp);
	}
}

/// @brief wrapper for relais_status
/// @param srv 
/// @param ctx 
/// @param rsp 
void relais_get(struct bt_mesh_onoff_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    struct bt_mesh_onoff_status *rsp)
{
	struct relais_ctx *relais = CONTAINER_OF(srv, struct relais_ctx, srv);

	relais_status(relais, rsp);
}


void relais_work(struct k_work *work)
{
	struct relais_ctx *relais = CONTAINER_OF(work, struct relais_ctx, work.work);

	if (relais->remaining) {
		relais_transition_start(relais);
	} else {
		relais->relais_output(relais->value);

		/* Publish the new value at the end of the transition */
		struct bt_mesh_onoff_status status;

		relais_status(relais, &status);
		bt_mesh_onoff_srv_pub(&relais->srv, NULL, &status);
	}
}