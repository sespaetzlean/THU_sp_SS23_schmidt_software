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
    uint16_t target_lvl;
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
            const struct bt_mesh_model_transition *transition);


/// @brief set a specific move speed
/// @param button 
/// @param delta size of a step
/// @param per_ms time to wait in ms between steps
/// @param delay time to wait in ms before starting move transition
/// @return error code of set message (0 if successful)
int move_level(struct level_button *button, int16_t delta, uint32_t per_ms, uint32_t delay);



//!!!following is dirty implementation as onOff Model is not used!!!

/**
 * @brief The following code implements a control structure 
 * that can differ how long a button is pressed and either toggles the appliance on or off 
 * or increases / decreases (alternating) the level of the appliance.
 * 
 * This code provides 2 business functions:
 * one that shall be called when the button is pressed,
 * the other shall be called when the button is released
 * 
 * Before using it shall be initialised via onOff_dim_decider_init
 */

#define TIME_ONOFF_DIM_DECIDE_4_DIM 2000

struct onOff_dim_decider_data {
    struct level_button *button;
    /** When turning of. the last level is saved here to be able to restore later
     * is used in function: toggle_lvl_onOff
     * When currently on, this is set to 0
    */
    uint16_t last_lvl;
    /** work structure needed for decider function*/
    struct k_work_delayable dec_work;
    /** true when increase was done last, false when decrease was done last*/
    bool last_increased;
    /** timestamp needed for calculation how long button was pressed*/
    int64_t timestamp;
    /** flag to save is pressed or released was last to not execute this twice*/
    bool last_pressed;
    /**the functions that are needed */
};

/// @brief initialize the onOff_dim_decider_data struct
/// @param data empty struct that shall be used for storing
/// @param button the client that shall be controlled
void onOff_dim_decider_init(struct onOff_dim_decider_data *data, 
            struct level_button *button);


/// @brief function that shall be called when the button is pressed
/// @param data 
void onOff_dim_decider_pressed(struct onOff_dim_decider_data *data);


/// @brief function that shall be called when the button is released
/// @param data 
void onOff_dim_decider_released(struct onOff_dim_decider_data *data);





#ifdef __cplusplus
}
#endif

#endif /* LVL_CLI_MOD_H__ */
