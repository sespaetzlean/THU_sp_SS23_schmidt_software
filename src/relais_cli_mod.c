#include "relais_cli_mod.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(relais_cli_mod, LOG_LEVEL_DBG);





void status_handler(struct bt_mesh_onoff_cli *cli,
			   struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_onoff_status *status)
{
	struct button *button = CONTAINER_OF(cli, struct button, client);
	button->status = status->present_on_off;
	LOG_DBG("Button %d: Received response: %s\n", button->pinNumber, status->present_on_off ? "on" : "off");
}



static int ack_unack_handler(struct button *button, struct bt_mesh_msg_ctx *ctx, struct bt_mesh_onoff_set * set, struct bt_mesh_onoff_status *rsp)
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
			err = bt_mesh_onoff_cli_set(&button->client, ctx, set, rsp);
			LOG_DBG("sent ACK command with bool %s", set->on_off ? "on":"off");
		} else {
			err = bt_mesh_onoff_cli_set_unack(&button->client, ctx, set);
			LOG_DBG("sent UNack command with bool %s", set->on_off ? "on":"off");
			if (!err) {
				/* There'll be no response status for the
				 * unacked message. Set the state immediately.
				 */
				button->status = set->on_off;
            }
		}
		if (err) {
			LOG_WRN("OnOff toggle failed: %d\n", err);
		}
		return err;
}


int toggle_onoff(struct button *button) {
		LOG_DBG("Toggle is executed");
		struct bt_mesh_onoff_set set = {
			.on_off = !(button->status),
		};

	return ack_unack_handler(button, NULL, &set, NULL);
}

int set_onoff(struct button *button, bool onOff) {
		LOG_DBG("Set is executed");
		struct bt_mesh_onoff_set set = {
			.on_off = onOff,
		};

	return ack_unack_handler(button, NULL, &set, NULL);
}


void button_handler_cb(uint32_t pressed, uint32_t changed)
{
	if (!bt_mesh_is_provisioned()) {
		return;
	}
	for (int i = 0; i < ARRAY_SIZE(buttons); ++i) {
		if (!(pressed & changed & BIT(buttons[i]->pinNumber))) {
			continue;
		}
		toggle_onoff(buttons[i]);
	}
}