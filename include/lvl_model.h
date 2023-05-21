/**
 * @file
 * @brief Model handler
 */

#ifndef LEVEL_MODEL_H__
#define LEVEL_MODEL_H__

#include <zephyr/bluetooth/mesh.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>

#ifdef __cplusplus
extern "C" {
#endif

struct dimmable_ctx {
	struct bt_mesh_lvl_srv srv;	//server instance (should include current OnOff-Value)
	struct k_work_delayable work;	//what should be done next (will be scheduled), so what should happen with the relais
	uint32_t remaining;				//remaining time until operation should be executed
	int16_t value;
	int16_t target_value;						//target/future value of the relais
	int pinNumber;
};

/// @brief set state of a dimmable element
/// @param srv server instance	(should include current OnOff-Value)
/// @param ctx context information for received message, (source, destination, ...)
/// @param set new onOff Status
/// @param rsp used to store the response
void dimmable_set(struct bt_mesh_lvl_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    const struct bt_mesh_lvl_set *set,
		    struct bt_mesh_lvl_status *rsp);

/// @brief get state of a dimmable element
/// @param srv server instance	(should include current OnOff-Value)
/// @param ctx context information for received message, (source, destination, ...)
/// @param rsp used to store the response
void dimmable_get(struct bt_mesh_lvl_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    struct bt_mesh_lvl_status *rsp);

/// @brief 
/// @param work 
void dimmable_work(struct k_work * work);

#ifdef __cplusplus
}
#endif

#endif /* LEVEL_MODEL_H__ */
