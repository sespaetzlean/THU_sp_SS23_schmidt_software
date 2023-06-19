#include "model_handler.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(models,LOG_LEVEL_DBG);


// ===================== temperature watchdog ====================== //

enum cmd_2_register_temp_watchdog {
	SWITCH_RELAIS1_CMD_TW,
	SET_DIMMER1_CMD_TW,
};

extern struct temp_watchdog_ctx temperature_watchdog;

//more definitions later before model construction

// ============================= IO definitions ============================= //
// === pwm definitions === //
#define DIMMER0_NODE	DT_ALIAS(dimmeroutput)


#if DT_NODE_HAS_STATUS(DIMMER0_NODE, okay)
static const struct pwm_dt_spec pwm0_spec = PWM_DT_SPEC_GET(DIMMER0_NODE);
#else
#error "Unsupported board: dimmer-output devicetree alias is not defined"
#endif


/// @brief wrapper function as this definition is needed 
/// for the dimmable_srv_ctx struct
/// @param pwmValue value the led_out shall be set to.
static void dimmer0_safe_setWrapper(uint16_t pwmValue)
{
	LOG_DBG("called dimmer0_safe_setWrapper with value %d on cmd %d", pwmValue, SET_DIMMER1_CMD_TW);
	safely_switch_level(&temperature_watchdog, SET_DIMMER1_CMD_TW, pwmValue);
}



// === gpio out definition === //
#define RELAIS1_NODE    DT_ALIAS(relaisoutput)

#if DT_NODE_HAS_STATUS(RELAIS1_NODE, okay)
static const struct gpio_dt_spec led1_spec = GPIO_DT_SPEC_GET(RELAIS1_NODE, gpios);
#else
#error "Unsupported board: relais-output devicetree alias is not defined"
#endif


/// @brief wrapper function as this definition is needed 
/// for the relais_srv_ctx struct
/// @param onOff_value true = on, false = off
static void relais1_safe_setWrapper(const bool onOff_value)
{
	LOG_DBG("called relais1_safe_setWrapper with value %d on cmd %d", onOff_value, SWITCH_RELAIS1_CMD_TW);
	safely_switch_onOff(&temperature_watchdog, 
		SWITCH_RELAIS1_CMD_TW, 
		onOff_value);
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





// ============================= model definitions ========================== //





// ===================== relais initializations ==============================//
static const struct bt_mesh_onoff_srv_handlers onoff_handlers = {
	.set = relais_set,
	.get = relais_get,
};

static struct relais_srv_ctx myRelais_ctx = { 
	.srv = BT_MESH_ONOFF_SRV_INIT(&onoff_handlers), 
	.relais_output = relais1_safe_setWrapper,
};





// ============ lvl initializations ----------------------------------------- //
static const struct bt_mesh_lvl_srv_handlers lvl_handlers = {
	.set = dimmable_set,
	.get = dimmable_get,
	.move_set = dimmable_move_set,
};

static struct dimmable_srv_ctx myDimmable_ctx = { 
	.srv = BT_MESH_LVL_SRV_INIT(&lvl_handlers), 
	.pwm_output = dimmer0_safe_setWrapper,
};





// ====== lightness initializations ----------------------------------------- //
/*
static const struct bt_mesh_lightness_srv_handlers lightness_srv_handlers = {
	.light_set = light_set,
	.light_get = light_get,
};

static struct lightness_ctx myLightness_ctx = {
	.srv = BT_MESH_LIGHTNESS_SRV_INIT(&lightness_srv_handlers),
	.pwm_output = dimmer0_safe_setWrapper,
};
*/





// =========== relais client initializations ================================ //
static struct relais_cli_ctx relais0_ctr = { 
	.client = BT_MESH_ONOFF_CLI_INIT(&relais_status_handler), 
};

/// @brief callback for relais0_ctr to toggle an OnOff-Server
/// @param port 
/// @param cb 
/// @param pins 
static void button0_risingEdge_cb(const struct device *port,
			struct gpio_callback *cb,
			gpio_port_pins_t pins)
{
	LOG_DBG("Callback of %s button rising edge activated", port->name);
	toggle_onoff(&relais0_ctr);
}

/// @brief callback for a switch/lever to turn ON relais0_ctr OnOff-Server
/// @param port 
/// @param cb 
/// @param pins 
static void lever_risingEdge_cb(const struct device *port,
			struct gpio_callback *cb,
			gpio_port_pins_t pins)
{
	LOG_DBG("Callback of %s rising edge activated", port->name);
	set_onoff(&relais0_ctr, true);
}
/// @brief callback for a switch/lever to turn OFF relais0_ctr OnOff-Server
/// @param port 
/// @param cb 
/// @param pins 
static void lever_fallingEdge_cb(const struct device *port,
			struct gpio_callback *cb,
			gpio_port_pins_t pins)
{
	LOG_DBG("Callback of %s button falling edge activated", port->name);
	set_onoff(&relais0_ctr, false);
}





// ==================== level client initializations ======================== //

static struct onOff_dim_decider_data decider_data;
static struct dimmable_cli_ctx level0_ctr = {
	.client = BT_MESH_LVL_CLI_INIT(&level_status_handler),
};

static void sw1_risingEdge_cb(const struct device *port,
			struct gpio_callback *cb,
			gpio_port_pins_t pins)
{
	LOG_DBG("Callback of %s button rising edge activated", port->name);
	move_level(&level0_ctr, 0, 0, 0);
}


// ! for implementation of dim_decider !
static void sw2_risingEdge_cb(const struct device *port,
			struct gpio_callback *cb,
			gpio_port_pins_t pins)
{
	LOG_DBG("Callback of %s button rising edge activated", port->name);
	onOff_dim_decider_pressed(&decider_data);
}

static void sw3_fallingEdge_cb(const struct device *port,
			struct gpio_callback *cb,
			gpio_port_pins_t pins)
{
	LOG_DBG("Callback of %s button falling edge activated", port->name);
	onOff_dim_decider_released(&decider_data);
}

// !ending implementation of dim_decider !






// === for relais model === //
static int execute_relais1_set_wrapper(bool value)
{
	LOG_DBG("execute_relais1_set_wrapper: %d", value);
	if(value) 
		gpio_pin_set_dt(&led1_spec, 1);
	else
		gpio_pin_set_dt(&led1_spec, 0);
	return 0;
}

static struct output_command relais1_cmd = {
	.cmd_type = OUTPUT_COMMAND_TYPE_ONOFF,
	.off_value = false,
	.gpio_set = execute_relais1_set_wrapper,
};

// === for level model === //
static int execute_dimmer1_set_wrapper(uint16_t value)
{
	LOG_DBG("execute_dimmer1_set_wrapper: %d", value);
	lc_pwm_output_set(&pwm0_spec, value);
	return 0;
}

static struct output_command dimmer1_cmd = {
	.cmd_type = OUTPUT_COMMAND_TYPE_LEVEL,
	.off_level = 0,
	.pwm_set = execute_dimmer1_set_wrapper,
};










// =================================== health service ======================= //		

static const struct bt_mesh_health_srv_cb health_srv_cb = {
	.attn_on = attention_on,
	.attn_off = attention_off,
};

static struct bt_mesh_health_srv health_srv = {
	.cb = &health_srv_cb,
};

BT_MESH_HEALTH_PUB_DEFINE(health_pub, 0);










// ========================== model definitions ============================= //
// === actor model === //
static struct bt_mesh_model std_relais_models[] = {
	//standard configuration server model 
	//that every node has in its first element
	BT_MESH_MODEL_CFG_SRV,		
	//same applies to the health model: every node has this in its 1. element
	BT_MESH_MODEL_HEALTH_SRV(&health_srv, &health_pub),	
	BT_MESH_MODEL_ONOFF_SRV(&myRelais_ctx.srv),
};

static struct bt_mesh_model std_dimmer_models[] = {
	BT_MESH_MODEL_LVL_SRV(&myDimmable_ctx.srv),
};

/*
static struct bt_mesh_model std_lightness_models[] = {
	BT_MESH_MODEL_LIGHTNESS_SRV(&myLightness_ctx.srv),
};
*/


// === sensor models === //
static struct bt_mesh_model relais_sensor_models[] = {
	BT_MESH_MODEL_ONOFF_CLI(&relais0_ctr.client),
};
static struct bt_mesh_model level_sensor_models[] = {
	BT_MESH_MODEL_LVL_CLI(&level0_ctr.client),
};



//exp: insert all elements the node consists of
//location descriptor is used to number the elements 
//in case there are multiple elements of same kind
static struct bt_mesh_elem elements[] = {
	BT_MESH_ELEM(1, std_relais_models, BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(2, std_dimmer_models, BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(3, relais_sensor_models, BT_MESH_MODEL_NONE),
	BT_MESH_ELEM(4, level_sensor_models, BT_MESH_MODEL_NONE),
	// //BT_MESH_ELEM(5, std_lightness_models, BT_MESH_MODEL_NONE),
};

/// @brief compose the node
static const struct bt_mesh_comp comp = {
	//set id that is defined in the prj.conf-file
	.cid = CONFIG_BT_COMPANY_ID,	
	.elem = elements,
	.elem_count = ARRAY_SIZE(elements),
};





const struct bt_mesh_comp *model_handler_init(void)
{
	int err = 0;
	//init attention
	err += abs(attention_init());


	//init IO first
	err += abs(single_device_init(pwm0_spec.dev));	//pwm out pin

	err += abs(single_device_init(led1_spec.port));	//gpio out pin
	//gpio out pin
	err += abs(gpio_pin_configure_dt(&led1_spec, GPIO_OUTPUT_INACTIVE));	
	
	err += abs(single_device_init(sw0_spec.port));	//gpio in pin
	err += abs(configure_gpi_interrupt(&sw0_spec, 
		GPIO_INT_EDGE_TO_ACTIVE, 
		&sw0_cb_data, 
		button0_risingEdge_cb));

	err += abs(single_device_init(sw1_spec.port));	//gpio in pin
	err += abs(configure_gpi_interrupt(&sw1_spec, 
		GPIO_INT_EDGE_TO_ACTIVE, 
		&sw1_cb_data, 
		sw1_risingEdge_cb));

	err += abs(single_device_init(sw2_spec.port));	//gpio in pin
	err += abs(configure_gpi_interrupt(&sw2_spec, 
		GPIO_INT_EDGE_TO_ACTIVE, 
		&sw2_cb_data, 
		sw2_risingEdge_cb));
	//!!! short P0.15 & P0.16 for this!!!
	err += abs(single_device_init(sw3_spec.port));	//gpio in pin
	err += abs(configure_gpi_interrupt(&sw3_spec, 
		GPIO_INT_EDGE_TO_INACTIVE, 
		&sw3_cb_data, 
		sw3_fallingEdge_cb));		

	if (0 != err) {
		LOG_ERR("Error during GPIO-initialization");
	} else {
		LOG_INF("all GPIO-Inits successful");
	}

	//init decider stuff
	onOff_dim_decider_init(&decider_data, &level0_ctr);

	//add safely_execute_functions to temperature watchdog
	register_output_cmd(&temperature_watchdog, 
				&relais1_cmd, 
				SWITCH_RELAIS1_CMD_TW);
	register_output_cmd(&temperature_watchdog, 
				&dimmer1_cmd, 
				SET_DIMMER1_CMD_TW);

	// === add all work_items to scheduler === //
	k_work_init_delayable(&myRelais_ctx.work, relais_work);
	k_work_init_delayable(&myDimmable_ctx.work, dimmable_work);
	// // k_work_init_delayable(&myLightness_ctx.work, lightness_work);

	return &comp;
}
