#include "lvl_cli_mod.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(level_cli_mod, LOG_LEVEL_DBG);


/// @brief helper to read a status message and save to dimmable_cli_ctx struct
/// @param c_ctx struct to write to
/// @param status status to read from
static void read_dimmable_status(struct dimmable_cli_ctx *c_ctx, 
            const struct bt_mesh_lvl_status *status)
{
    c_ctx->target_lvl = mesh_level2struct_level(status->target);
	c_ctx->current_lvl = mesh_level2struct_level(status->current);
    LOG_DBG("Read STATUS 2 cli struct. cur %d, tar %d", 
		c_ctx->current_lvl, 
		c_ctx->target_lvl);
}





void level_status_handler(struct bt_mesh_lvl_cli *cli,
            struct bt_mesh_msg_ctx *ctx,
            const struct bt_mesh_lvl_status *status)
{
    struct dimmable_cli_ctx *c_ctx = 
		CONTAINER_OF(cli, struct dimmable_cli_ctx, client);
    read_dimmable_status(c_ctx, status);
    LOG_DBG("Cli recv STATUS");
}





/// @brief check whether it is a unicast or group address, 
/// send ack or unack command
/// @param c_ctx dimmable_cli_ctx struct
/// @param ctx message context
/// @param set desired level
/// @param rsp save response here
/// @return error code of the set message (0 when successful)
static int ack_unack_set_handler(struct dimmable_cli_ctx *c_ctx, 
            struct bt_mesh_msg_ctx *ctx, 
            struct bt_mesh_lvl_set * set, 
            struct bt_mesh_lvl_status *rsp)
{
    int err;
    /* As we can't know how many nodes are in a group, it doesn't
	* make sense to send acknowledged messages to group addresses -
	* we won't be able to make use of the responses anyway. (This also
	* applies in LPN mode, since we can't expect to receive a response
	* in appropriate time).
	*/
	if (bt_mesh_model_pub_is_unicast(c_ctx->client.model)) 
	{
		err = bt_mesh_lvl_cli_set(&c_ctx->client, ctx, set, rsp);
		LOG_DBG("Cli sent ACK cmd: lvl %d", set->lvl);
	} else {
		err = bt_mesh_lvl_cli_set_unack(&c_ctx->client, ctx, set);
		LOG_DBG("Cli sent UNack cmd: lvl %d", set->lvl);
		if (!err) {
			/* There'll be no response status for the
			 * unacked message. Set the state immediately.
			 */
            struct bt_mesh_lvl_status artificial_response = {
                .target = set->lvl,
            };
            read_dimmable_status(c_ctx, &artificial_response);
        }
	}
	if (err) {
		LOG_WRN("Lvl setting failed: %d\n", err);
	}
	return err;
}





/// @brief check whether it is a unicast or group address, 
/// send ack or unack command
/// @param c_ctx dimmable_cli_ctx struct
/// @param ctx message context
/// @param set desired move speed
/// @param rsp save response here
/// @return error code of the set message (0 when successful)
static int ack_unack_move_handler(struct dimmable_cli_ctx *c_ctx, 
            struct bt_mesh_msg_ctx *ctx, 
            struct bt_mesh_lvl_move_set * set, 
            struct bt_mesh_lvl_status *rsp)
{
    int err;
    /* As we can't know how many nodes are in a group, it doesn't
	* make sense to send acknowledged messages to group addresses -
	* we won't be able to make use of the responses anyway. (This also
	* applies in LPN mode, since we can't expect to receive a response
	* in appropriate time).
	*/
	if (bt_mesh_model_pub_is_unicast(c_ctx->client.model)) 
	{
		err = bt_mesh_lvl_cli_move_set(&c_ctx->client, ctx, set, rsp);
		LOG_DBG("Cli sent ACK cmd: move step %d, pause %d", 
			set->delta, 
			set->transition->time);
	} else {
		err = bt_mesh_lvl_cli_move_set_unack(&c_ctx->client, ctx, set);
		LOG_DBG("Cli sent UNack cmd: move step %d, pause %d", 
			set->delta, 
			set->transition->time);
        //TODO
        //exp no artificial response here, as level moves anyway
		//? how to get the actual level of the appliance
	}
	if (err) {
		LOG_WRN("Lvl moving failed: %d\n", err);
	}
	return err;
}





int set_level(struct dimmable_cli_ctx *c_ctx, 
			uint16_t level,
            const struct bt_mesh_model_transition *transition)
{
    LOG_DBG("SET lvl is executed");
    struct bt_mesh_lvl_set set = {
        .lvl = struct_level2mesh_level(level),
        .transition = transition,
    };

    return ack_unack_set_handler(c_ctx, NULL, &set, NULL);
}





int move_level(struct dimmable_cli_ctx *c_ctx, 
			int16_t delta, 
			uint32_t per_ms, 
			uint32_t delay)
{
    LOG_DBG("MOVE lvl is executed");
	struct bt_mesh_model_transition tempTransition = {
		.time = per_ms,
		.delay = delay,
	};
    struct bt_mesh_lvl_move_set set = {
        .delta = delta,
		.transition = &tempTransition,
    };

    return ack_unack_move_handler(c_ctx, NULL, &set, NULL);
}





//!!!following is dirty implementation as onOff Model is not used!!!

/// @brief helper to decide whether to increase or decrease the level 
/// based on the last_increased flag in onOff_dim_decider_data 
/// or client_ctx data respectively
/// @param data 
/// @return error code of set message (0 if successful)
static int inc_dec_lvl_decider(struct onOff_dim_decider_data *data)
{
	/** use the toggle if the level is in between,
	 * if level is max or min, always de- or increase respectively
	*/
	int16_t delta;
	if(0 == data->client_ctx->current_lvl) {
		data->last_increased = false;
	} else if (UINT16_MAX == data->client_ctx->current_lvl) {
		data->last_increased = true;
	}

	if(data->last_increased)
	{
		//now decrease
		data->last_increased = false;
		delta = -1024;
	} else {
		//now increase
		data->last_increased = true;
		delta = 1024;
	}
	return move_level(data->client_ctx, delta, 100, 0);
}





/// @brief toggle the appliance on or off. 
/// @param c_ctx dimmable_cli_ctx struct
/// @param transition transition struct to set delay or transition time
/// @return error code of set message (0 if successful)
static int toggle_lvl_onOff(struct onOff_dim_decider_data *dec_data, 
			const struct bt_mesh_model_transition *transition)
{
	/** first check if led is on or off, 
	 * if on, save this level and turn it off
	 * if off, turn it on to last saved level
	 */
	uint16_t target_lvl;
	uint16_t currentLevel = dec_data->client_ctx->current_lvl;
	if(0 == currentLevel) {
		LOG_DBG("dirty toggles to ON");
		/** it may happen that last saved level is 0
		 * Then this one should not be used 
		 * and instead appliance turned on fully
		 */
		if(0 == dec_data->last_lvl) {
			target_lvl = UINT16_MAX;
		} else {
			//turn on to last saved level 
			// & del last_lvl (that this one is not used again)
			target_lvl = dec_data->last_lvl;
		}
		dec_data->last_lvl = 0;
	} else {
		//turn off
		LOG_DBG("dirty toggles to OFF");
		dec_data->last_lvl = currentLevel;
		target_lvl = 0;
	}
	LOG_DBG("dirty TOGGLE lvl to %d is executed", target_lvl);
	return set_level(dec_data->client_ctx, target_lvl, transition);
}




/// @brief work item used for scheduling dimming
/// @param work 
static void onOff_dim_decider_work_handler(struct k_work *work)
{
	struct onOff_dim_decider_data *data = 
		CONTAINER_OF(work, struct onOff_dim_decider_data, dec_work);
	inc_dec_lvl_decider(data);
}





//public functions

void onOff_dim_decider_init(struct onOff_dim_decider_data *data, 
            struct dimmable_cli_ctx *c_ctx)
{
	data->last_pressed = false;
	data->client_ctx = c_ctx;
	//when first toggle is executed, appliance will be set to full level
	data->last_lvl = UINT16_MAX;	
	data->last_increased = false;	//shall be increased first
	//add work to scheduler
	k_work_init_delayable(&data->dec_work, onOff_dim_decider_work_handler);
	LOG_INF("Decider initialised");
}


void onOff_dim_decider_pressed(struct onOff_dim_decider_data *data)
{
	if(data->last_pressed) {
		return;
	}
	data->last_pressed = true;
	//schedule the work item.
	//dimming shall start after 500 ms
	k_work_reschedule(&data->dec_work, K_MSEC(TIME_ONOFF_DIM_DECIDE_4_DIM));
	//save timestamp for future calculation how long button has been pressed
	data->timestamp = k_uptime_get();
}


void onOff_dim_decider_released(struct onOff_dim_decider_data *data)
{
	if(!data->last_pressed) {
		return;
	}
	data->last_pressed = false;
	/**depending on when the button was pressed:
	 * before TIME_ONOFF_DIM_DECIDE_4_DIM: toggle on/off
	 * after TIME_ONOFF_DIM_DECIDE_4_DIM: stop dim up/down
	 */

	//check when button was pressed
	int64_t time_pressed = k_uptime_delta(&data->timestamp) + 10;		
	//+ 10 ms to be sure work did not start yet
	//TODO: remove 0
	LOG_DBG("Button %d was pressed for %lld ms", 0, time_pressed);
	if(time_pressed < TIME_ONOFF_DIM_DECIDE_4_DIM) {
		k_work_cancel_delayable(&data->dec_work);
		//TODO: check return code
		toggle_lvl_onOff(data, NULL);
	} else {
		//stop moving
		/**
		 * here, always the acknowledged command shall be executed
		 * This is needed as for the next operation, the helper struct has to know the current values
		 * in order to decide on the right next action
		 * ! This may lead to a lot of ack messages if the group is large !
		 */
		struct bt_mesh_model_transition tempTran = {
			.delay = 0,
			.time = 0,
		};
		struct bt_mesh_lvl_move_set tempSet = {
			.delta = 0,
			.transition = &tempTran,
		};
		LOG_DBG("Move stopped with ACK-com");
		bt_mesh_lvl_cli_move_set(&data->client_ctx->client, NULL, &tempSet, NULL);
		//TODO: check return code
	}
}