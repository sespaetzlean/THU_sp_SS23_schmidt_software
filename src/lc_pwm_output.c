#include "lc_pwm_output.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(pwm_out,LOG_LEVEL_DBG);


#define PWM_PERIOD 1024     //microseconds it takes for one pwm cycle





int single_device_init(const struct device *dev)
{
	if (!device_is_ready(dev)) {
		LOG_ERR("Error: Device %s is not ready\n", dev->name);
        return -1;
	} else {
        LOG_INF("Device %s ready", dev->name);
        return 0;
    }
}





void lc_pwm_output_set(const struct pwm_dt_spec *pwm_spec, 
			const uint16_t desired_lvl)
{
	uint32_t scaled_lvl =
		(PWM_PERIOD * desired_lvl) /
		BT_MESH_LIGHTNESS_MAX;

	pwm_set_dt(pwm_spec, PWM_USEC(PWM_PERIOD), PWM_USEC(scaled_lvl));
}


int configure_gpi_interrupt(const struct gpio_dt_spec *spec, 
			gpio_flags_t flags, 
			struct gpio_callback *callback, 
			gpio_callback_handler_t handler)
{
	int err = 0;
	err += gpio_pin_configure_dt(spec, GPIO_INPUT);	//must be gpio in pin
	err += gpio_pin_interrupt_configure_dt(spec, flags);
	gpio_init_callback(callback, handler, BIT(spec->pin));
	err += gpio_add_callback(spec->port, callback);
	return err;
}
