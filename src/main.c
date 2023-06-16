#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include "model_handler.h"


#include <zephyr/bluetooth/mesh.h>
#include <zephyr/drivers/hwinfo.h>
#include "gpio_pwm_helpers.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(MAIN,LOG_LEVEL_DBG);

// ======================== Temperature watchdog ============================ //
#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	ADC_DT_SPEC_GET_BY_IDX(node_id, idx),


static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
			     DT_SPEC_AND_COMMA)
};

static struct adc_channel_ctx adc_ctx;





// ================ provisioning model ====================================== //
/** quick and dirty implementation of a provisioning model. 
 * This model mainly generates a semi random uuid*/

static uint8_t dev_uuid[16];

static const struct bt_mesh_prov prov = {
	.uuid = dev_uuid,
};

static const struct bt_mesh_prov *bt_mesh_MY_prov_init(void)
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
	//TODO: set dcdc enable pin high

	//TODO: init and schedule constant temperature check
	err = adc_pin_init(&adc_channels[0], &adc_ctx);
	if (err != 0) {
		LOG_ERR("ADC init failed (err %d)\n", err);
		return;
	}
	int16_t adc_value = read_adc_digital(&adc_ctx);
	LOG_WRN("First ADC val: %d", adc_value);

	LOG_DBG("Initializing...\n");

	err = bt_enable(bt_ready);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)\n", err);
	}
}