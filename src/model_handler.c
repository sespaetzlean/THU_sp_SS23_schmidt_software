/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include <dk_buttons_and_leds.h>
#include "model_handler.h"

#include "lvl_model.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(models,LOG_LEVEL_DBG);

/// @brief control state of a relais
/// @param srv server instance	(should include current OnOff-Value)
/// @param ctx context information for received message, (source, destination, ...)
/// @param set new onOff Status
/// @param rsp used to store the response
static void relais_set(struct bt_mesh_onoff_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    const struct bt_mesh_onoff_set *set,
		    struct bt_mesh_onoff_status *rsp);

/// @brief get state of a relais
/// @param srv server instance	(should include current OnOff-Value)
/// @param ctx context information for received message, (source, destination, ...)
/// @param rsp used to store the response
static void relais_get(struct bt_mesh_onoff_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    struct bt_mesh_onoff_status *rsp);

static const struct bt_mesh_onoff_srv_handlers onoff_handlers = {
	.set = relais_set,
	.get = relais_get,
};

struct relais_ctx {
	struct bt_mesh_onoff_srv srv;	//server instance (should include current OnOff-Value)
	struct k_work_delayable work;	//what should be done next (will be scheduled), so what should happen with the relais
	uint32_t remaining;				//remaining time until operation should be executed
	bool value;						//target/future value of the relais
	int pinNumber;
};

//initialise the OnOff-Model
//this is a list in case there will be multiple elements
static struct relais_ctx myRelais_ctx = { .srv = BT_MESH_ONOFF_SRV_INIT(&onoff_handlers), .pinNumber = 0};


/// @brief schedules the relais toggle
/// @param relais the relais that should be toggled
static void relais_transition_start(struct relais_ctx *relais)
{
	//exp As long as the transition is in progress, the onoff state shall be "on"
	dk_set_led(relais->pinNumber, true);
	k_work_reschedule(&relais->work, K_MSEC(relais->remaining));	//the work will be scheduled again after the "remaining"-value
	relais->remaining = 0;		//so remaining can be set to 0 now
}

/// @brief get the current status (including transition time) and save it in the status parameter
/// @param relais the relais that should be queried
/// @param status here the current status will be saved
static void relais_status(const struct relais_ctx *relais, struct bt_mesh_onoff_status *status)
{
	/* Do not include delay in the remaining time. */
	status->remaining_time = relais->remaining ? relais->remaining :	
		k_ticks_to_ms_ceil32(k_work_delayable_remaining_get(&relais->work));	//in case relais->remaining is ZERO (which can happen when relais_transition_start was executed), it is checked for when this execution is scheduled)
	status->target_on_off = relais->value;
	/* As long as the transition is in progress, the onoff state is "on": */
	status->present_on_off = relais->value || status->remaining_time;
}



/// @brief set a (future) status for the relais
/// @param srv server instance	(should include current OnOff-Value)
/// @param ctx context information for received message, (source, destination, ...)
/// @param set new state that should be set
/// @param rsp give in case you want a status response after set
static void relais_set(struct bt_mesh_onoff_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    const struct bt_mesh_onoff_set *set,
		    struct bt_mesh_onoff_status *rsp)
{
	struct relais_ctx *relais = CONTAINER_OF(srv, struct relais_ctx, srv);

	//when future value == current value
	if (set->on_off == relais->value) {
		goto respond;
	}

	relais->value = set->on_off;
	if (!bt_mesh_model_transition_time(set->transition)) {
		//execute if the transition time is already 0
		relais->remaining = 0;
		dk_set_led(relais->pinNumber, set->on_off);
		goto respond;
	}

	relais->remaining = set->transition->time;

	if (set->transition->delay) {
		k_work_reschedule(&relais->work, K_MSEC(set->transition->delay));
	} else {
		relais_transition_start(relais);
	}

respond:
	if (rsp) {	//only send response if a status pointer was given. Use the before declared function
		relais_status(relais, rsp);
	}
}

/// @brief wrapper for relais_status
/// @param srv 
/// @param ctx 
/// @param rsp 
static void relais_get(struct bt_mesh_onoff_srv *srv, struct bt_mesh_msg_ctx *ctx,
		    struct bt_mesh_onoff_status *rsp)
{
	struct relais_ctx *relais = CONTAINER_OF(srv, struct relais_ctx, srv);

	relais_status(relais, rsp);
}


static void relais_work(struct k_work *work)
{
	struct relais_ctx *relais = CONTAINER_OF(work, struct relais_ctx, work.work);

	if (relais->remaining) {
		relais_transition_start(relais);
	} else {
		dk_set_led(relais->pinNumber, relais->value);

		/* Publish the new value at the end of the transition */
		struct bt_mesh_onoff_status status;

		relais_status(relais, &status);
		bt_mesh_onoff_srv_pub(&relais->srv, NULL, &status);
	}
}














/// lvl gedöns ----------------------------------------- //

// extern void (* dimmable_set)(struct bt_mesh_lvl_srv *srv, struct bt_mesh_msg_ctx *ctx,
// 		    const struct bt_mesh_lvl_set *set,
// 		    struct bt_mesh_lvl_status *rsp);

// extern void (* dimmable_get)(struct bt_mesh_lvl_srv *srv, struct bt_mesh_msg_ctx *ctx,
// 		    struct bt_mesh_lvl_status *rsp);
//TODO: initialize model

static const struct bt_mesh_lvl_srv_handlers lvl_handlers = {
	.set = dimmable_set,
	.get = dimmable_get,
};


static struct dimmable_ctx myDimmable_ctx = { .srv = BT_MESH_LVL_SRV_INIT(&lvl_handlers), .pinNumber = 0 };






















// =================================================================================================== //
// =================================================================================================== //
// ===================================== attention service =========================================== //
// =================================================================================================== //

/* Set up a repeating delayed work to blink the DK's LEDs when attention is
 * requested.
 */
static struct k_work_delayable attention_blink_work;
static bool attention;

static void attention_blink(struct k_work *work)
{
	static int idx;
	const uint8_t pattern[] = {
#if DT_NODE_EXISTS(DT_ALIAS(led0))
		BIT(0),
#endif
#if DT_NODE_EXISTS(DT_ALIAS(led1))
		BIT(1),
#endif
#if DT_NODE_EXISTS(DT_ALIAS(led2))
		BIT(2),
#endif
#if DT_NODE_EXISTS(DT_ALIAS(led3))
		BIT(3),
#endif
	};

	if (attention) {
		dk_set_leds(pattern[idx++ % ARRAY_SIZE(pattern)]);
		k_work_reschedule(&attention_blink_work, K_MSEC(30));
	} else {
		dk_set_leds(DK_NO_LEDS_MSK);
	}
}

static void attention_on(struct bt_mesh_model *mod)
{
	attention = true;
	k_work_reschedule(&attention_blink_work, K_NO_WAIT);
}

static void attention_off(struct bt_mesh_model *mod)
{
	/* Will stop rescheduling blink timer */
	attention = false;
}

// ===================================== health service ============================================== //
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
	//BT_MESH_MODEL_ONOFF_SRV(&myRelais_ctx.srv),
	BT_MESH_MODEL_LVL_SRV(&myDimmable_ctx.srv),

};

//exp: insert all elements the node consists of
//location descriptor is used to number the elements in case there are multiple elements of same kind
static struct bt_mesh_elem elements[] = {
	BT_MESH_ELEM(1, std_relais_models, BT_MESH_MODEL_NONE),
	//BT_MESH_ELEM(2, dimmable_models, BT_MESH_MODEL_NONE),
};

/// @brief compose the node
static const struct bt_mesh_comp comp = {
	.cid = CONFIG_BT_COMPANY_ID,	//set id that is defined in the prj.conf-file
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

const struct bt_mesh_comp *model_handler_init(void)
{
	k_work_init_delayable(&attention_blink_work, attention_blink);

	//k_work_init_delayable(&myRelais_ctx.work, relais_work);

	k_work_init_delayable(&myDimmable_ctx.work, dimmable_work);


	return &comp;
}
