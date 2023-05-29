/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

/** @file
 *  @brief Nordic mesh light sample
 */
#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
//#include <bluetooth/mesh/dk_prov.h>
// #include <dk_buttons_and_leds.h>
#include "model_handler.h"


#include <zephyr/bluetooth/mesh.h>
#include <zephyr/drivers/hwinfo.h>

static uint8_t dev_uuid[16];

/// dirty prov instance. No OOB is supported
static const struct bt_mesh_prov prov = {
	.uuid = dev_uuid,
};

//dirty prov init. No OOB is supported
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


static void bt_ready(int err)
{
	if (err) {
		printk("Bluetooth LE-Stack init failed (err %d)\n", err);
		return;
	}

	printk("Bluetooth LE-Stack initialized\n");


	err = bt_mesh_init(bt_mesh_MY_prov_init(), model_handler_init());	//model_handler_init has to be defined before!!!
	if (err) {
		printk("Initializing mesh failed (err %d)\n", err);
		return;
	}

	//check if device saves provisioning data. If yes, load that data
	if (IS_ENABLED(CONFIG_SETTINGS)) {
		settings_load();
	}

	/* This will be a no-op if settings_load() loaded provisioning info */
	bt_mesh_prov_enable(BT_MESH_PROV_ADV | BT_MESH_PROV_GATT);

	printk("Mesh initialized\n");
}

void main(void)
{
	int err;

	printk("Initializing...\n");

	err = bt_enable(bt_ready);
	if (err) {
		printk("Bluetooth init failed (err %d)\n", err);
	}
}
