/**
 * @file
 * @brief Model handler
 */

#ifndef LIGHTNESS_MODEL_H__
#define LIGHTNESS_MODEL_H__

#include <zephyr/bluetooth/mesh.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>


#ifdef __cplusplus
extern "C" {
#endif

#define PWM_SIZE_STEP 512		
//by how much the pwm value should be changed each step 
//(so changing light always uses this step size that will be called more or less often, 
//depending how drastically light has to change)

struct lightness_ctx {
	struct bt_mesh_lightness_srv srv;
	struct k_work_delayable work;
	uint16_t target_lvl;
	uint16_t current_lvl;
	uint32_t time_period;       //total time for light to change from current_lvl to target_lvl
	uint32_t remaining_time;    //time until transition (including delay) is finished
	void (*pwm_output)(uint16_t level);	//function pointer to execute set value
};


/// @brief callback to handle incoming messages for lightness set opcode
// will execute start_new_light_transition and give a status update in the rsp
/// @param srv lightness server instance
/// @param ctx context of the message (not used)
/// @param set how the light should look in future (level and transition)
/// @param rsp status response message
void light_set(struct bt_mesh_lightness_srv *srv,
		      struct bt_mesh_msg_ctx *ctx,
		      const struct bt_mesh_lightness_set *set,
		      struct bt_mesh_lightness_status *rsp);


/// @brief callback to handle incoming messages for lightness get opcode
/// @param srv lightness server instance
/// @param ctx context of the message (not used)
/// @param rsp status response message
void light_get(struct bt_mesh_lightness_srv *srv, 
                struct bt_mesh_msg_ctx *ctx,
		        struct bt_mesh_lightness_status *rsp);




void lightness_work(struct k_work *work);

#ifdef __cplusplus
}
#endif

#endif /* LIGHTNESS_MODEL_H__ */
