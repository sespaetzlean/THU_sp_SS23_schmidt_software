#include "model_handler.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(models,LOG_LEVEL_DBG);

//exp: choose relais type
// #define __MONOSTABLE_RELAIS
#define __BISTABLE_RELAIS
#define RELAIS_SET_TIME_MS 21

//exp: choose type of button: button / lever 
//exp: & if relais or dimmer shall be controlled
// #define __DIMM_CTR_BY_BUTTON
#define __RELAIS_CTR_BY_BUTTON
// #define __RELAIS_CTR_BY_LEVER



#if defined __MONOSTABLE_RELAIS && defined __BISTABLE_RELAIS
#error "Only one relais type shall be defined"
#endif

#if defined __RELAIS_CTR_BY_LEVER && defined __RELAIS_CTR_BY_BUTTON
#error "Only one type of button shall be defined"
#endif

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
static uint16_t dimmer0_safe_setWrapper(uint16_t pwmValue)
{
	LOG_DBG("called dimmer0_safe_setWrapper with value %d on cmd %d", 
		pwmValue, SET_DIMMER1_CMD_TW);
	return safely_switch_level(&temperature_watchdog, SET_DIMMER1_CMD_TW, pwmValue);
}





// === gpio out definition === //
#define RELAIS1_NODE_SET    DT_ALIAS(relaiscloseoutput)
#if DT_NODE_HAS_STATUS(RELAIS1_NODE_SET, okay)
static const struct gpio_dt_spec out1_setTog_spec = 
	GPIO_DT_SPEC_GET(RELAIS1_NODE_SET, gpios);
#else
#error "Unsupported board: relaiscloseoutput devicetree alias is not defined"
#endif

#if defined __BISTABLE_RELAIS
	#define RELAIS1_NODE_UNSET    DT_ALIAS(relaisopenoutput)
	#if DT_NODE_HAS_STATUS(RELAIS1_NODE_UNSET, okay)
	static const struct gpio_dt_spec out1_unset_spec = 
		GPIO_DT_SPEC_GET(RELAIS1_NODE_UNSET, gpios);
	#else
	#error "Unsupported board: relaisopenoutput devicetree alias is not defined"
	#endif
#endif

/// @brief wrapper function as this definition is needed 
/// for the relais_srv_ctx struct
/// @param onOff_value true = on, false = off
static bool relais1_safe_setWrapper(const bool onOff_value)
{
	LOG_DBG("called relais1_safe_setWrapper with value %d on cmd %d", 
		onOff_value, 
		SWITCH_RELAIS1_CMD_TW);
	return safely_switch_onOff(&temperature_watchdog, 
		SWITCH_RELAIS1_CMD_TW, 
		onOff_value);
}





// === gpio in definition with interrupt === //
#if defined __DIMM_CTR_BY_BUTTON
	//!same input needs to be connected to both of these pins
	#define IN0_A_NODE     DT_ALIAS(dimmonoffbuttonrisingedge)
	#if DT_NODE_HAS_STATUS(IN0_A_NODE, okay)
	static const struct gpio_dt_spec input0a_spec = 
		GPIO_DT_SPEC_GET(IN0_A_NODE, gpios);
	#else
	#error "Unsupported board: dimmonoffbuttonrisingedge devicetree alias is not defined"
	#endif
	static struct gpio_callback input0a_cb_data;

	#define IN0_B_NODE     DT_ALIAS(dimmonoffbuttonfallingedge)
	#if DT_NODE_HAS_STATUS(IN0_B_NODE, okay)
	static const struct gpio_dt_spec input0b_spec = 
		GPIO_DT_SPEC_GET(IN0_B_NODE, gpios);
	#else
	#error "Unsupported board: dimmonoffbuttonfallingedge devicetree alias is not defined"
	#endif
	static struct gpio_callback input0b_cb_data;
#endif

// ===
#if defined __RELAIS_CTR_BY_LEVER
	//!same input needs to be connected to both of these pins
	#define IN1_A_NODE     DT_ALIAS(relaisleverrisingedge)
	#if DT_NODE_HAS_STATUS(IN1_A_NODE, okay)
	static const struct gpio_dt_spec input1a_spec = 
		GPIO_DT_SPEC_GET(IN1_A_NODE, gpios);
	#else
	#error "Unsupported board: dimmonoffbuttonrisingedge devicetree alias is not defined"
	#endif
	static struct gpio_callback input1a_cb_data;

	#define IN1_B_NODE     DT_ALIAS(relaisleverfallingedge)
	#if DT_NODE_HAS_STATUS(IN1_B_NODE, okay)
	static const struct gpio_dt_spec input1b_spec = 
		GPIO_DT_SPEC_GET(IN1_B_NODE, gpios);
	#else
	#error "Unsupported board: dimmonoffbuttonfallingedge devicetree alias is not defined"
	#endif
	static struct gpio_callback input1b_cb_data;
#elif defined __RELAIS_CTR_BY_BUTTON
	#define IN2_NODE     DT_ALIAS(relaistogglebutton)
	#if DT_NODE_HAS_STATUS(IN2_NODE, okay)
	static const struct gpio_dt_spec input2_spec = 
		GPIO_DT_SPEC_GET(IN2_NODE, gpios);
	#else
	#error "Unsupported board: relaistogglebutton devicetree alias is not defined"
	#endif
	static struct gpio_callback input2_cb_data;
#endif




// ========================================================================== //
// ============================= model definitions ========================== //
// ========================================================================== //

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
static void relais0_button_risEdge_cb(const struct device *port,
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
static void relais0_lever_risEdge_cb(const struct device *port,
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
static void relais0_lever_falEdge_cb(const struct device *port,
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


// ! for implementation of dim_decider !
static void dimDcdr_risEdge_cb(const struct device *port,
			struct gpio_callback *cb,
			gpio_port_pins_t pins)
{
	LOG_DBG("Callback of %s button rising edge activated", port->name);
	onOff_dim_decider_pressed(&decider_data);
}

static void dimDcdr_falEdge_cb(const struct device *port,
			struct gpio_callback *cb,
			gpio_port_pins_t pins)
{
	LOG_DBG("Callback of %s button falling edge activated", port->name);
	onOff_dim_decider_released(&decider_data);
}
// !ending implementation of dim_decider !






// === for relais model === //
static bool execute_relais1_set_wrapper(bool value)
{
	LOG_DBG("execute_relais1_set_wrapper: %d", value);
	//monostable and bistable relais types are controlled differently
	#if defined __MONOSTABLE_RELAIS
		if(value) 
		{
			gpio_pin_set_dt(&out1_setTog_spec, 1);
		}
		else {
			gpio_pin_set_dt(&out1_setTog_spec, 0);
		}
	#elif defined __BISTABLE_RELAIS
		if(value) 
		{
			gpio_pin_set_dt(&out1_setTog_spec, 1);
			k_msleep(RELAIS_SET_TIME_MS);
			gpio_pin_set_dt(&out1_setTog_spec, 0);
		}
		else {
			gpio_pin_set_dt(&out1_unset_spec, 1);
			k_msleep(RELAIS_SET_TIME_MS);
			gpio_pin_set_dt(&out1_unset_spec, 0);
		}
	#else
		#error "The board type has not been defined"
	#endif
	return value;
}

static void update_relais1_state_wrapper(bool current_value)
{
	LOG_DBG("update_relais1_state_wrapper: %d", current_value);
	relais_update(&myRelais_ctx, current_value, 0);
}

static struct output_command relais1_cmd = {
	.cmd_type = OUTPUT_COMMAND_TYPE_ONOFF,
	.off_value = false,
	.gpio_set = execute_relais1_set_wrapper,
	.gpio_update = update_relais1_state_wrapper,
};

// === for level model === //
static uint16_t execute_dimmer1_set_wrapper(uint16_t value)
{
	LOG_DBG("execute_dimmer1_set_wrapper: %d", value);
	lc_pwm_output_set(&pwm0_spec, value);
	return value;
}

static void update_dimmer1_state_wrapper(uint16_t current_level)
{
	LOG_DBG("update_dimmer1_state_wrapper: %d", current_level);
	dimmable_update(&myDimmable_ctx, current_level, current_level, 0);
}

static struct output_command dimmer1_cmd = {
	.cmd_type = OUTPUT_COMMAND_TYPE_LEVEL,
	.off_level = 0,
	.pwm_set = execute_dimmer1_set_wrapper,
	.pwm_update = update_dimmer1_state_wrapper,
};









// ========================================================================== //
// ==================== health service ====================================== //
// ========================================================================== //

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




// === Put all models together ... === ///
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

	//gpio out pins
	err += abs(single_device_init(out1_setTog_spec.port));	//gpio out pin
	err += abs(gpio_pin_configure_dt(&out1_setTog_spec, GPIO_OUTPUT_INACTIVE));
#if defined __BISTABLE_RELAIS
	err += abs(single_device_init(out1_unset_spec.port));	//gpio out pin
	err += abs(gpio_pin_configure_dt(&out1_unset_spec, GPIO_OUTPUT_INACTIVE));
#endif //ifdef __BISTABLE_RELAIS

	//gpio in pins
#if defined __DIMM_CTR_BY_BUTTON
	err += abs(single_device_init(input0a_spec.port));	//gpio in pin
	err += abs(configure_gpi_interrupt(&input0a_spec, 
		GPIO_INT_EDGE_TO_ACTIVE, 
		&input0a_cb_data, 
		dimDcdr_risEdge_cb));
	err += abs(single_device_init(input0b_spec.port));	//gpio in pin
	err += abs(configure_gpi_interrupt(&input0b_spec, 
		GPIO_INT_EDGE_TO_INACTIVE, 
		&input0b_cb_data, 
		dimDcdr_falEdge_cb));
#endif //ifdef __DIMM_CTR_BY_BUTTON
#if defined __RELAIS_CTR_BY_BUTTON
	err += abs(configure_gpi_interrupt(&input2_spec, 
		GPIO_INT_EDGE_TO_ACTIVE, 
		&input2_cb_data, 
		relais0_button_risEdge_cb));
#elif defined __RELAIS_CTR_BY_LEVER
	err += abs(single_device_init(input1a_spec.port));	//gpio in pin
	err += abs(configure_gpi_interrupt(&input1a_spec, 
		GPIO_INT_EDGE_TO_ACTIVE, 
		&input1a_cb_data, 
		relais0_lever_risEdge_cb));
	err += abs(single_device_init(input1b_spec.port));	//gpio in pin
	err += abs(configure_gpi_interrupt(&input1b_spec, 
		GPIO_INT_EDGE_TO_INACTIVE, 
		&input1b_cb_data, 
		relais0_lever_falEdge_cb));
#endif //relais input type

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
