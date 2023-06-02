#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include "model_handler.h"


#include <zephyr/bluetooth/mesh.h>
#include <zephyr/drivers/hwinfo.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MAIN,LOG_LEVEL_DBG);





// ================ provisioning model ====================================== //
/** quick and dirty implementation of a provisioning model. 
 * This model mainly generates a semi random uuid*/

static uint8_t dev_uuid[16];

static const struct bt_mesh_prov prov = {
	.uuid = dev_uuid,
};

const struct bt_mesh_prov *bt_mesh_MY_prov_init(void)
{
	/* Generate an RFC-4122 version 4 compliant UUID.
	See dk_prov.c for details*/
	size_t id_len = hwinfo_get_device_id(dev_uuid, sizeof(dev_uuid));
	if (!IS_ENABLED(CONFIG_BT_MESH_DK_LEGACY_UUID_GEN)) {
		for (size_t i = id_len; i < sizeof(dev_uuid); i++) {
			dev_uuid[i] = dev_uuid[i % id_len] ^ 0xff;
		}
	}
	dev_uuid[6] = (dev_uuid[6] & BIT_MASK(4)) | BIT(6);
	dev_uuid[8] = (dev_uuid[8] & BIT_MASK(6)) | BIT(7);
	return &prov;
}











/// @brief function to set up the whole bluetooth mesh stack
/// @param err error code of the function that calls this function
static void bt_ready(int err)
{
	if (err) {
		LOG_ERR("Bluetooth LE-Stack init failed (err %d)\n", err);
		return;
	}

	LOG_INF("Bluetooth LE-Stack initialized\n");


	err = bt_mesh_init(bt_mesh_MY_prov_init(), model_handler_init());
	if (err) {
		LOG_ERR("Initializing mesh failed (err %d)\n", err);
		return;
	}

	//check if device saved provisioning data before. If yes, load that data
	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	//exp: This will not be executed if settings_load() loaded provisioning info
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

	LOG_INF("Mesh initialized\n");
}






// ============================ MAIN ======================================== //
void main(void)
{
	int err;

	LOG_DBG("Initializing...\n");

	err = bt_enable(bt_ready);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
	}
}
