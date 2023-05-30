/*
 * Copyright (c) 2019 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include "model_handler.h"

#include "health_model.h"
#include "relais_model.h"
#include "lvl_model.h"
#include "lightness_model.h"
// =====================
#include "relais_cli_mod.h"
#include "lvl_cli_mod.h"

#include"lc_pwm_output.h"					//for pwm output

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(models,LOG_LEVEL_DBG);


// ============================= IO definitions ============================= //
// === pwm definitions === //
#define PWM_OUT0_NODE	DT_ALIAS(pwm_led0)

#if DT_NODE_HAS_STATUS(PWM_OUT0_NODE, okay)
static const struct pwm_dt_spec pwm0_spec = PWM_DT_SPEC_GET(PWM_OUT0_NODE);
#else
#error "Unsupported board: pwm-out0 devicetree alias is not defined"
#endif

/// @brief wrapper function as this definition is needed for the dimmable_ctx struct
/// @param pwmValue value the led_out shall be set too.
static void pwm0_setWrapper(uint16_t pwmValue)
{
	lc_pwm_output_set(&pwm0_spec, pwmValue);
}



// === gpio out definition === //
#define LED1_NODE    DT_ALIAS(led1)

#if DT_NODE_HAS_STATUS(LED1_NODE, okay)
static const struct gpio_dt_spec led1_spec = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
#else
#error "Unsupported board: led1 devicetree alias is not defined"
#endif

/// @brief wrapper function as this definition is needed for the relais_ctx struct
/// @param onOff_value true = on, false = off
static void led1_setWrapper(const bool onOff_value)
{
	if(onOff_value == true)
		gpio_pin_set_dt(&led1_spec, 1);
	else
		gpio_pin_set_dt(&led1_spec, 0);
}



// === gpio in definition with interrupt === //
#define SW0_NODE     DT_ALIAS(sw0)
#if DT_NODE_HAS_STATUS(SW0_NODE, okay)
static const struct gpio_dt_spec sw0_spec = GPIO_DT_SPEC_GET(SW0_NODE, gpios);
#else
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
static struct gpio_callback sw0_cb_data;

#define SW1_NODE     DT_ALIAS(sw1)
#if DT_NODE_HAS_STATUS(SW1_NODE, okay)
static const struct gpio_dt_spec sw1_spec = GPIO_DT_SPEC_GET(SW1_NODE, gpios);
#else
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif
static struct gpio_callback sw1_cb_data;

#define SW2_NODE     DT_ALIAS(sw2)
#if DT_NODE_HAS_STATUS(SW2_NODE, okay)
static const struct gpio_dt_spec sw2_spec = GPIO_DT_SPEC_GET(SW2_NODE, gpios);
#else
#error "Unsupported board: sw2 devicetree alias is not defined"
#endif
static struct gpio_callback sw2_cb_data;

#define SW3_NODE     DT_ALIAS(sw3)
#if DT_NODE_HAS_STATUS(SW3_NODE, okay)
static const struct gpio_dt_spec sw3_spec = GPIO_DT_SPEC_GET(SW3_NODE, gpios);
#else
#error "Unsupported board: sw3 devicetree alias is not defined"
#endif
static struct gpio_callback sw3_cb_data;












// relais gedöns ============
static const struct bt_mesh_onoff_srv_handlers onoff_handlers = {
	.set = relais_set,
	.get = relais_get,
};

static struct relais_ctx myRelais_ctx = { 
	.srv = BT_MESH_ONOFF_SRV_INIT(&onoff_handlers), 
	.relais_output = led1_setWrapper
};





/// lvl gedöns ----------------------------------------- //
static const struct bt_mesh_lvl_srv_handlers lvl_handlers = {
	.set = dimmable_set,
	.get = dimmable_get,
	.move_set = dimmable_move_set,
};


static struct dimmable_ctx myDimmable_ctx = { 
	.srv = BT_MESH_LVL_SRV_INIT(&lvl_handlers), 
	.pwm_output = pwm0_setWrapper,
};






/// lightness gedöns ----------------------------------------- //
static const struct bt_mesh_lightness_srv_handlers lightness_srv_handlers = {
	.light_set = light_set,
	.light_get = light_get,
};


static struct lightness_ctx myLightness_ctx = {
	//TODO
	.srv = BT_MESH_LIGHTNESS_SRV_INIT(&lightness_srv_handlers),
	.pwm_output = pwm0_setWrapper,
};






// =========== relais client gedöns ================= //
static struct relais_button button0 = { 
	.client = BT_MESH_ONOFF_CLI_INIT(&relais_status_handler), 
};

static void sw0_risingEdge_cb(const struct device *port,
			struct gpio_callback *cb,
			gpio_port_pins_t pins)
{
	LOG_DBG("Callback of %d button rising edge activated", sw0_spec.pin);
	toggle_onoff(&button0);
}



// =========== level client gedöns ================== //

static struct onOff_dim_decider_data decider_data;
static struct level_button button1 = {
	.client = BT_MESH_LVL_CLI_INIT(&level_status_handler),
};

static void sw1_risingEdge_cb(const struct device *port,
			struct gpio_callback *cb,
			gpio_port_pins_t pins)
{
	LOG_DBG("Callback of %d button rising edge activated", sw1_spec.pin);
	move_level(&button1, 0, 0, 0);
}

static void sw2_risingEdge_cb(const struct device *port,
			struct gpio_callback *cb,
			gpio_port_pins_t pins)
{
	LOG_DBG("Callback of %d button rising edge activated", sw2_spec.pin);
	// move_level(&button1, 1024, 100, 0);
	onOff_dim_decider_pressed(&decider_data);
}

static void sw3_risingEdge_cb(const struct device *port,
			struct gpio_callback *cb,
			gpio_port_pins_t pins)
{
	LOG_DBG("Callback of %d button rising edge activated", sw3_spec.pin);
	// move_level(&button1, -1024, 100, 0);
	onOff_dim_decider_released(&decider_data);
}









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







// === aktor model === //
static struct bt_mesh_model std_relais_models[] = {
	BT_MESH_MODEL_CFG_SRV,		//standard configuration server model that every node has in its first element
	BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),	//same applies to the health model: every node has in its first element
	//TODO: === Select right model ===
	//BT_MESH_MODEL_ONOFF_SRV(&myRelais_ctx.srv),
	BT_MESH_MODEL_LVL_SRV(&myDimmable_ctx.srv),
	// BT_MESH_MODEL_LIGHTNESS_SRV(&myLightness_ctx.srv),

};

// === sensor models === //
static struct bt_mesh_model relais_sensor_models[] = {
	BT_MESH_MODEL_ONOFF_CLI(&button0.client),
};
static struct bt_mesh_model level_sensor_models[] = {
	BT_MESH_MODEL_LVL_CLI(&button1.client),
};

//exp: insert all elements the node consists of
//location descriptor is used to number the elements in case there are multiple elements of same kind
static struct bt_mesh_elem elements[] = {
	BT_MESH_ELEM(1, std_relais_models, BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(2, relais_sensor_models, BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(3, level_sensor_models, BT_MESH_MODEL_NONE),
};

/// @brief compose the node
static const struct bt_mesh_comp comp = {
	.cid = CONFIG_BT_COMPANY_ID,	//set id that is defined in the prj.conf-file
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};

const struct bt_mesh_comp *model_handler_init(void)
{
	int err = 0;
	//init IO first
	err += single_device_init(pwm0_spec.dev);	//pwm out pin

	err += single_device_init(led1_spec.port);	//gpio out pin
	err += gpio_pin_configure_dt(&led1_spec, GPIO_OUTPUT_INACTIVE);	//gpio out pin
	
	err += single_device_init(sw0_spec.port);	//gpio in pin
	err *= configure_gpi_interrupt(&sw0_spec, GPIO_INT_EDGE_TO_ACTIVE, &sw0_cb_data, sw0_risingEdge_cb);

	err += single_device_init(sw1_spec.port);	//gpio in pin
	err *= configure_gpi_interrupt(&sw1_spec, GPIO_INT_EDGE_TO_ACTIVE, &sw1_cb_data, sw1_risingEdge_cb);

	err += single_device_init(sw2_spec.port);	//gpio in pin
	err *= configure_gpi_interrupt(&sw2_spec, GPIO_INT_EDGE_TO_ACTIVE, &sw2_cb_data, sw2_risingEdge_cb);

	err += single_device_init(sw3_spec.port);	//gpio in pin
	err *= configure_gpi_interrupt(&sw3_spec, GPIO_INT_EDGE_TO_ACTIVE, &sw3_cb_data, sw3_risingEdge_cb);

	if (0 != err) {
		LOG_ERR("Error while initializing IO");
	} else {
		LOG_INF("all IO initialized");
	}

	//init decider stuff
	onOff_dim_decider_init(&decider_data, &button1);

	// === add all work_items to scheduler === //
	k_work_init_delayable(&attention_blink_work, attention_blink);
	//TODO: === select right model ===
	// k_work_init_delayable(&myRelais_ctx.work, relais_work);
	k_work_init_delayable(&myDimmable_ctx.work, dimmable_work);
	// k_work_init_delayable(&myLightness_ctx.work, lightness_work);

	return &comp;
}
