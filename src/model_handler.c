/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include <dk_buttons_and_leds.h>
#include "model_handler.h"

#include "health_model.h"

#include "relais_model.h"
#include "lvl_model.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(models,LOG_LEVEL_DBG);

// ============================= pwm definitions ============================= //
//TODO
// #define PWM_OUT0_NODE	DT_ALIAS(pwm_led0)

// #if DT_NODE_HAS_STATUS(PWM_OUT0_NODE, okay)
// static const struct pwm_dt_spec out0 = PWM_DT_SPEC_GET(PWM_OUT0_NODE);
// #else
// #error "Unsupported board: pwm-out0 devicetree alias is not defined"
// #endif





//relais gedöns ============
static const struct bt_mesh_onoff_srv_handlers onoff_handlers = {
	.set = relais_set,
	.get = relais_get,
};

static struct relais_ctx myRelais_ctx = { .srv = BT_MESH_ONOFF_SRV_INIT(&onoff_handlers), .pinNumber = 0};

















/// lvl gedöns ----------------------------------------- //
//TODO: initialize model

static const struct bt_mesh_lvl_srv_handlers lvl_handlers = {
	.set = dimmable_set,
	.get = dimmable_get,
};


static struct dimmable_ctx myDimmable_ctx = { .srv = BT_MESH_LVL_SRV_INIT(&lvl_handlers)};



























// =================================================================================================== //
// =================================================================================================== //
// ===================================== attention service =========================================== //
// =================================================================================================== //

/* Set up a repeating delayed work to blink the DK's LEDs when attention is
 * requested.
 */



// ===================================== health service ============================================== //
extern struct k_work_delayable attention_blink_work;		//is defined in health_model.c

static const struct bt_mesh_health_srv_cb health_srv_cb = {
	.attn_on = attention_on,
	.attn_off = attention_off,
};

static struct bt_mesh_health_srv health_srv = {
	.cb = &health_srv_cb,
};

BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);












static struct bt_mesh_model std_relais_models[] = {
	BT_MESH_MODEL_CFG_SRV,		//standard configuration server model that every node has in its first element
	BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),	//same applies to the health model: every node has in its first element
	BT_MESH_MODEL_ONOFF_SRV(&myRelais_ctx.srv),
	//BT_MESH_MODEL_LVL_SRV(&myDimmable_ctx.srv),

};

//exp: insert all elements the node consists of
//location descriptor is used to number the elements in case there are multiple elements of same kind
static struct bt_mesh_elem elements[] = {
	BT_MESH_ELEM(1, std_relais_models, BT_MESH_MODEL_NONE),
};

/// @brief compose the node
static const struct bt_mesh_comp comp = {
	.cid = CONFIG_BT_COMPANY_ID,	//set id that is defined in the prj.conf-file
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

const struct bt_mesh_comp *model_handler_init(void)
{
	//init pwm first
	//dimmable_init(list_all_pwm, ARRAY_SIZE(list_all_pwm));

	//add all work_items to scheduler
	k_work_init_delayable(&attention_blink_work, attention_blink);
	k_work_init_delayable(&myRelais_ctx.work, relais_work);
	//k_work_init_delayable(&myDimmable_ctx.work, dimmable_work);


	return &comp;
}
