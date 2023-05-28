/**
 * @file
 * @brief Model handler
 */

#ifndef LEVEL_MODEL_H__
#define LEVEL_MODEL_H__

#include <zephyr/bluetooth/mesh.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>

//TODO
//#include "lc_pwm_output.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PWM_SIZE_STEP 512		
//by how much the pwm value should be changed each step 
//(so changing light always uses this step size that will be called more or less often, 
//depending how drastically light has to change)

struct dimmable_ctx {
	struct bt_mesh_lvl_srv srv;		//server instance (should include current OnOff-Value)
	struct k_work_delayable work;	//what should be done next (will be scheduled), so what should happen with the relais
	uint32_t remaining_time;		//remaining time until operation is finished
	uint32_t time_period;			//how long to wait in between the steps of size: PWM_SIZE_STEP
	uint16_t current_lvl;
	uint16_t target_lvl;			//target/future value of the relais
	int32_t delta_lvl; 				//how fast the light shall be changed, (step size per ms)
	void (*pwm_output)(uint16_t level);	//function pointer to execute set value
};

/// @brief set state of a dimmable element
/// @param srv server instance	(should include current OnOff-Value)
/// @param ctx context information for received message, (source, destination, ...)
/// @param set new Level Status value
/// @param rsp used to store the response
void dimmable_set(struct bt_mesh_lvl_srv *srv,
			struct bt_mesh_msg_ctx *ctx,
		    const struct bt_mesh_lvl_set *set,
		    struct bt_mesh_lvl_status *rsp);


/// @brief get state of a dimmable element
/// @param srv server instance	(should include current OnOff-Value)
/// @param ctx context information for received message, (source, destination, ...)
/// @param rsp used to store the response
void dimmable_get(struct bt_mesh_lvl_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    struct bt_mesh_lvl_status *rsp);


/// @brief set state of a dimmable element
/// @param srv server instance	(should include current OnOff-Value)
/// @param ctx context information for received message, (source, destination, ...)
/// @param delta_set speed of the change (steps / ms)
/// @param rsp used to store the response
void dimmable_delta_set(struct bt_mesh_lvl_srv *srv, 
			struct bt_mesh_msg_ctx *ctx,
			const struct bt_mesh_lvl_delta_set *delta_set,
			struct bt_mesh_lvl_status *rsp)


/// @brief periodically calls itself until target_lvl is reached
/// @param work 
void dimmable_work(struct k_work *work);

#ifdef __cplusplus
}
#endif

#endif /* LEVEL_MODEL_H__ */
