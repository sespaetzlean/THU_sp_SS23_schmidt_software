/**
 * @file
 * @brief Model handler
 */

#ifndef RELAIS_MODEL_H__
#define RELAIS_MODEL_H__

#include <zephyr/bluetooth/mesh.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>


#ifdef __cplusplus
extern "C" {
#endif


struct relais_ctx {
	struct bt_mesh_onoff_srv srv;	//server instance (should include current OnOff-Value)
	struct k_work_delayable work;	//what should be done next (will be scheduled), so what should happen with the relais
	uint32_t remaining;				//remaining time until operation should be executed
	bool value;						//target/future value of the relais
	int pinNumber;
};


/// @brief control state of a relais
/// @param srv server instance	(should include current OnOff-Value)
/// @param ctx context information for received message, (source, destination, ...)
/// @param set new onOff Status
/// @param rsp used to store the response
void relais_set(struct bt_mesh_onoff_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    const struct bt_mesh_onoff_set *set,
		    struct bt_mesh_onoff_status *rsp);


/// @brief get state of a relais
/// @param srv server instance	(should include current OnOff-Value)
/// @param ctx context information for received message, (source, destination, ...)
/// @param rsp used to store the response
void relais_get(struct bt_mesh_onoff_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    struct bt_mesh_onoff_status *rsp);


void relais_work(struct k_work *work);

#ifdef __cplusplus
}
#endif

#endif /* RELAIS_MODEL_H__ */
