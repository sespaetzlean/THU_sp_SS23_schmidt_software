/**
 * @file
 * @brief Model handler
 */

#ifndef LVL_CLI_MOD_H__
#define LVL_CLI_MOD_H__

#include <zephyr/bluetooth/mesh.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/mesh/proxy.h>
#include <bluetooth/mesh/models.h>

#include "conversion_helpers.h"


#ifdef __cplusplus
extern "C" {
#endif

struct level_button {
	/** Current level status of the corresponding server. */
    uint16_t current_lvl;
    /** Target level status of the corresponding server*/
    uint16_t target_level;
	/** Generic OnOff client instance for this switch. */
	struct bt_mesh_lvl_cli client;
};


/// @brief for handling the incoming status messages
/// @param cli 
/// @param ctx 
/// @param status 
void level_status_handler(struct bt_mesh_lvl_cli *cli,
            struct bt_mesh_msg_ctx *ctx,
            const struct bt_mesh_lvl_status *status);


/// @brief set a specific level 
/// @param button 
/// @param level
/// @param transition transition struct to set delay or transition time
/// @return error code of set message (0 if successful)
int set_level(struct level_button *button, 
            uint16_t level,
            struct bt_mesh_model_transition *transition);


/// @brief set a specific move speed
/// @param button 
/// @param delta size of a step
/// @param per_ms time to wait between steps
/// @return error code of set message (0 if successful)
int move_level(struct level_button *button, int16_t delta, uint32_t per_ms, uint32_t delay);


void onOff_dimm_decider(struct level_button *button);


#ifdef __cplusplus
}
#endif

#endif /* LVL_CLI_MOD_H__ */
