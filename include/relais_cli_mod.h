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


struct relais_cli_ctx {
	/** Current light status of the corresponding server. */
	bool status;
	/** Generic OnOff client instance for this switch. */
	struct bt_mesh_onoff_cli client;
};


/// @brief save incoming status in button struct
/// @param cli instance of the client
/// @param ctx message context
/// @param status status to read from
void relais_status_handler(struct bt_mesh_onoff_cli *cli,
			   struct bt_mesh_msg_ctx *ctx,
			   const struct bt_mesh_onoff_status *status);


/// @brief 
/// @param button 
/// @return error code of set message (0 if successful)
int toggle_onoff(struct relais_cli_ctx *button);


/// @brief 
/// @param button 
/// @param onOff true = on, false = off 
/// @return error code of set message (0 if successful)
int set_onoff(struct relais_cli_ctx *button, bool onOff);


#ifdef __cplusplus
}
#endif

#endif /* RELAIS_CLI_MOD_H__ */
