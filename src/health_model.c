#include "health_model.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(att_mod,LOG_LEVEL_DBG);


#define INFO_LED_NODE    DT_ALIAS(info0led)

#if DT_NODE_HAS_STATUS(INFO_LED_NODE, okay)
static const struct gpio_dt_spec info_led_spec = GPIO_DT_SPEC_GET(INFO_LED_NODE, gpios);
#else
#error "Unsupported board: info_led devicetree alias is not defined"
#endif


static bool attention;
struct k_work_delayable attention_blink_work;


void attention_blink(struct k_work *work)
{
	if (attention) {
		gpio_pin_toggle_dt(&info_led_spec);
		k_work_reschedule(&attention_blink_work, K_MSEC(300));
	} else {
		gpio_pin_set_dt(&info_led_spec, 0);
		LOG_INF("Attention turned of");
	}
}

void attention_on(struct bt_mesh_model *mod)
{
	attention = true;
	LOG_WRN("Ready to provision!");
	k_work_reschedule(&attention_blink_work, K_NO_WAIT);
}

void attention_off(struct bt_mesh_model *mod)
{
	/* Will stop rescheduling blink timer */
	attention = false;
}

int attention_init()
{
	int err = single_device_init(&info_led_spec);
	k_work_init_delayable(&attention_blink_work, attention_blink);
	return err;
}