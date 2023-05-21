#include "lc_pwm_output.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(pwm_out,LOG_LEVEL_DBG);


#define PWM_PERIOD 1024     //microseconds it takes for one pwm cycle


int lc_pwm_output_init(struct device *pwm_dev)
{
	if (!device_is_ready(pwm_dev)) {
		LOG_ERR("Error: PWM device %s is not ready\n", pwm_dev->name);
        return -1;
	} else {
        LOG_DBG("PWM %s ready", pwm_dev->name);
        return 0;
    }
}


void lc_pwm_output_set(struct device *pwm_dev, uint16_t desired_lvl)
{
	uint32_t scaled_lvl =
		(PWM_PERIOD * desired_lvl) /
		BT_MESH_LIGHTNESS_MAX;

	pwm_set_dt(&pwm_dev, PWM_USEC(PWM_PERIOD), PWM_USEC(scaled_lvl));
}
