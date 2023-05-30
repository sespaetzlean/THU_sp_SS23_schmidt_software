#include "lvl_cli_mod.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(level_cli_mod, LOG_LEVEL_DBG);


/// @brief helper to read a status message and save to level_button struct
/// @param button struct to write to
/// @param status status to read from
static void read_dimmable_status(struct level_button *button, 
            const struct bt_mesh_lvl_status *status)
{
    button->target_lvl = mesh_level2struct_level(status->target);
	button->current_lvl = mesh_level2struct_level(status->current);
    LOG_DBG("Read status to client struct");
}





void level_status_handler(struct bt_mesh_lvl_cli *cli,
            struct bt_mesh_msg_ctx *ctx,
            const struct bt_mesh_lvl_status *status)
{
    struct level_button *button = CONTAINER_OF(cli, struct level_button, client);
    read_dimmable_status(button, status);
    LOG_DBG("Cli rx target_lvl %d ", button->target_lvl);
}





/// @brief check whether it is a unicast or group address, send ack or unack command
/// @param button level_button struct
/// @param ctx message context
/// @param set desired level
/// @param rsp save response here
/// @return error code of the set message (0 when successful)
static int ack_unack_set_handler(struct level_button *button, 
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
	if (bt_mesh_model_pub_is_unicast(button->client.model)) 
	{
		err = bt_mesh_lvl_cli_set(&button->client, ctx, set, rsp);
		LOG_DBG("Cli sent ACK cmd: lvl %d", set->lvl);
	} else {
		err = bt_mesh_lvl_cli_set_unack(&button->client, ctx, set);
		LOG_DBG("Cli sent UNack cmd: lvl %d", set->lvl);
		if (!err) {
			/* There'll be no response status for the
			 * unacked message. Set the state immediately.
			 */
            struct bt_mesh_lvl_status artificial_response = {
                .target = set->lvl,
            };
            read_dimmable_status(button, &artificial_response);
        }
	}
	if (err) {
		LOG_WRN("Lvl setting failed: %d\n", err);
	}
	return err;
}





/// @brief check whether it is a unicast or group address, send ack or unack command
/// @param button level_button struct
/// @param ctx message context
/// @param set desired move speed
/// @param rsp save response here
/// @return error code of the set message (0 when successful)
static int ack_unack_move_handler(struct level_button *button, 
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
	if (bt_mesh_model_pub_is_unicast(button->client.model)) 
	{
		err = bt_mesh_lvl_cli_move_set(&button->client, ctx, set, rsp);
		LOG_DBG("Cli sent ACK cmd: move step %d, pause %d", set->delta, set->transition->time);
	} else {
		err = bt_mesh_lvl_cli_move_set_unack(&button->client, ctx, set);
		LOG_DBG("Cli sent UNack cmd: move step %d, pause %d", set->delta, set->transition->time);
        //TODO
        //no artificial response here, level moves anyway
	}
	if (err) {
		LOG_WRN("Lvl moving failed: %d\n", err);
	}
	return err;
}





int set_level(struct level_button *button, 
			uint16_t level,
            struct bt_mesh_model_transition *transition)
{
    LOG_DBG("SET lvl is executed");
    struct bt_mesh_lvl_set set = {
        .lvl = struct_level2mesh_level(level),
        .transition = transition,
    };

    return ack_unack_set_handler(button, NULL, &set, NULL);
}





int move_level(struct level_button *button, int16_t delta, uint32_t per_ms, uint32_t delay)
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

    return ack_unack_move_handler(button, NULL, &set, NULL);
}





//!!!following is dirty implementation as onOff Model is not used!!!

/// @brief helper to decide whether to increase or decrease the level based on the last_increased flag in onOff_dim_decider_data
/// @param data 
/// @return error code of set message (0 if successful)
static int inc_dec_lvl_decider(struct onOff_dim_decider_data *data)
{
	if(data->last_increased)
	{
		//now decrease
		data->last_increased = false;
		return move_level(data->button, -1024, 100, 0);
	}
	//now increase
	data->last_increased = true;
	return move_level(data->button, 1024, 100, 0);
}





/// @brief toggle the appliance on or off. 
/// @param button level_button struct
/// @param transition transition struct to set delay or transition time
/// @return error code of set message (0 if successful)
static int toggle_lvl_onOff(struct onOff_dim_decider_data *dec_data, 
			const struct bt_mesh_model_transition *transition)
{
	uint16_t lastLevelBuffer = dec_data->last_lvl;
	if(0 == lastLevelBuffer)
	{
		//appliance is currently on
		dec_data->last_lvl = dec_data->button->target_lvl;
		LOG_DBG("dirty TOGGLE lvl to 000 is executed");
	} else {
		//appliance is currently off
		dec_data->last_lvl = 0;
		LOG_DBG("dirty TOGGLE lvl to %d is executed", lastLevelBuffer);
	}
	return set_level(dec_data->button, lastLevelBuffer, transition);
}




/// @brief work item used for scheduling dimming
/// @param work 
static void onOff_dim_decider_work_handler(struct k_work *work)
{
	struct onOff_dim_decider_data *data = CONTAINER_OF(work, struct onOff_dim_decider_data, dec_work);
	inc_dec_lvl_decider(data);
}





//public functions

void onOff_dim_decider_init(struct onOff_dim_decider_data *data, 
            const struct level_button *button)
{
	data->button = button;
	data->last_lvl = UINT16_MAX;	//when first toggle is executed, appliance will be set to full level
	data->last_increased = false;	//shall be increased first
	//add work to scheduler
	k_work_init_delayable(&data->dec_work, onOff_dim_decider_work_handler);
	LOG_INF("Decider initialised");
}


void onOff_dim_decider_pressed(struct onOff_dim_decider_data *data)
{
	//schedule the work item.
	//dimming shall start after 500 ms
	k_work_reschedule(&data->dec_work, K_MSEC(TIME_ONOFF_DIM_DECIDE_4_DIM));
	//save timestamp for future calculation how long button has been pressed
	data->timestamp = k_uptime_get();
}


void onOff_dim_decider_released(struct onOff_dim_decider_data *data)
{
	/**depending on when the button was pressed:
	 * before TIME_ONOFF_DIM_DECIDE_4_DIM: toggle on/off
	 * after TIME_ONOFF_DIM_DECIDE_4_DIM: stop dim up/down
	 */

	//check when button was pressed
	int64_t time_pressed = k_uptime_delta(&data->timestamp) + 10;		//+ 10 ms to be sure work did not start yet
	//TODO: remove 0
	LOG_DBG("Button %d was pressed for %lld ms", 0, time_pressed);
	if(time_pressed < TIME_ONOFF_DIM_DECIDE_4_DIM) {
		k_work_cancel_delayable(&data->dec_work);
		//TODO: check return code
		toggle_lvl_onOff(data, NULL);
	} else {
		//stop moving
		move_level(data->button, 0, 0, 0);
		//TODO: check return code
	}
}