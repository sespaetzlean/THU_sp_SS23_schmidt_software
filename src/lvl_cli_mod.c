#include "lvl_cli_mod.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(level_cli_mod, LOG_LEVEL_DBG);


/// @brief writes the incoming status message to the level_button struct
/// @param button struct to write to
/// @param status status to read from
static void level_status_writer(struct level_button *button, 
            const struct bt_mesh_lvl_status *status)
{
    button->target_level = input_level2bt_level(status->target);
    LOG_DBG("Created status message");
}





void level_status_handler(struct bt_mesh_lvl_cli *cli,
            struct bt_mesh_msg_ctx *ctx,
            const struct bt_mesh_lvl_status *status)
{
    struct level_button *button = CONTAINER_OF(cli, struct level_button, client);
    level_status_writer(button, status);
    LOG_DBG("Button: Received target %d ", button->target_level);
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
		LOG_DBG("sent ACK command with level %d", set->lvl);
	} else {
		err = bt_mesh_lvl_cli_set_unack(&button->client, ctx, set);
		LOG_DBG("sent UNack command with level %d", set->lvl);
		if (!err) {
			/* There'll be no response status for the
			 * unacked message. Set the state immediately.
			 */
            struct bt_mesh_lvl_status artificial_response = {
                .target = set->lvl,
            };
            level_status_writer(button, &artificial_response);
        }
	}
	if (err) {
		LOG_WRN("Level setting failed: %d\n", err);
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
		LOG_DBG("sent ACK command with move step %d", set->delta);
	} else {
		err = bt_mesh_lvl_cli_move_set_unack(&button->client, ctx, set);
		LOG_DBG("sent UNack command with move step %d", set->delta);
        //TODO
        //no artificial response here, level moves anyway
	}
	if (err) {
		LOG_WRN("Level moving failed: %d\n", err);
	}
	return err;
}





int set_level(struct level_button *button, 
			uint16_t level,
            struct bt_mesh_model_transition *transition)
{
    LOG_DBG("Level Set is executed");
    struct bt_mesh_lvl_set set = {
        .lvl = input_level2bt_level(level),
        .transition = transition,
    };

    return ack_unack_set_handler(button, NULL, &set, NULL);
}





int move_level(struct level_button *button, int16_t delta_speed)
{
    LOG_DBG("Level move is executed");
    struct bt_mesh_lvl_move_set set = {
        .delta = delta_speed,
    };

    return ack_unack_move_handler(button, NULL, &set, NULL);
}