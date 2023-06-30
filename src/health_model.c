#include "health_model.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(att_mod,LOG_LEVEL_DBG);


#define INFO_LED_NODE    DT_ALIAS(healthinfoled)

#if DT_NODE_HAS_STATUS(INFO_LED_NODE, okay)
static const struct gpio_dt_spec info_led_spec = GPIO_DT_SPEC_GET(INFO_LED_NODE, gpios);
#else
#error "Unsupported board: healthinfoled devicetree alias is not defined"
#endif


static bool attention;
static struct k_work_delayable attention_blink_work;


/// @brief work struct that executes tasks to attract attention
/// @param work 
static void attention_blink(struct k_work *work)
{
	if (attention) {
		gpio_pin_toggle_dt(&info_led_spec);
		k_work_reschedule(&attention_blink_work, K_MSEC(300));
		LOG_DBG("Blink");
	} else {
		gpio_pin_set_dt(&info_led_spec, 0);
		LOG_DBG("Stop blinking");
	}
}

void attention_on(struct bt_mesh_model *mod)
{
	attention = true;
	LOG_WRN("Attention!");
	k_work_reschedule(&attention_blink_work, K_NO_WAIT);
}

void attention_off(struct bt_mesh_model *mod)
{
	/* Will stop rescheduling blink timer */
	attention = false;
	LOG_INF("Attention turned off");
}

int attention_init()
{
	int err = abs(single_device_init(info_led_spec.port));
	err += abs(gpio_pin_configure_dt(&info_led_spec, GPIO_OUTPUT_INACTIVE));	
	k_work_init_delayable(&attention_blink_work, attention_blink);
	return err;
}