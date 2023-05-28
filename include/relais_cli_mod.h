/**
 * @file
 * @brief Model handler
 */

#ifndef RELAIS_CLI_MOD_H__
#define RELAIS_CLI_MOD_H__

#include <zephyr/bluetooth/mesh.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/mesh/proxy.h>
#include <bluetooth/mesh/models.h>


#ifdef __cplusplus
extern "C" {
#endif

extern struct button *buttons[1];

struct button {
	/** Current light status of the corresponding server. */
	bool status;
	/** Generic OnOff client instance for this switch. */
	struct bt_mesh_onoff_cli client;
	/** Number of the pin the button is connected to*/
	int pinNumber;
};


/// @brief save incoming status in button struct
/// @param cli instance of the client
/// @param ctx message context
/// @param status status to read from
void status_handler(struct bt_mesh_onoff_cli *cli,
			   struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_onoff_status *status);


/// @brief this callback will be executed whenever one of the button states is changed
/// @param pressed register that shows which buttons of the GPIO are currently pressed
/// @param changed register that shows which button states of the GPIO changed 
void button_handler_cb(uint32_t pressed, uint32_t changed);


#ifdef __cplusplus
}
#endif

#endif /* RELAIS_CLI_MOD_H__ */
