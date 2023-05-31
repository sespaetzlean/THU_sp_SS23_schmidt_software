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
	struct bt_mesh_onoff_srv srv;	//server instance
	struct k_work_delayable work;
	uint32_t remaining;				//remaining time until operation is finished
	bool value;				//present or future value of the relais
	//function pointer that executes onOff of appliance
	void (*relais_output)(bool onOFF_value);	
};


/// @brief control state of a relais
/// @param srv server instance
/// @param ctx context information for received message
/// @param set new onOff Status
/// @param rsp used to store the response
void relais_set(struct bt_mesh_onoff_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    const struct bt_mesh_onoff_set *set,
		    struct bt_mesh_onoff_status *rsp);


/// @brief get state of a relais
/// @param srv server instance
/// @param ctx context information for received message
/// @param rsp used to store the response
void relais_get(struct bt_mesh_onoff_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    struct bt_mesh_onoff_status *rsp);


/// @brief execute state setting in hardware
/// @param work 
void relais_work(struct k_work *work);

#ifdef __cplusplus
}
#endif

#endif /* RELAIS_MODEL_H__ */
