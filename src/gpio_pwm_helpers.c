#include "gpio_pwm_helpers.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(gpio_pwm_adc,LOG_LEVEL_DBG);


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



// ============================= PWM functions ============================== //

void lc_pwm_output_set(const struct pwm_dt_spec *pwm_spec, 
			const uint16_t desired_lvl)
{
	uint32_t scaled_lvl =
		(PWM_PERIOD * desired_lvl) /
		BT_MESH_LIGHTNESS_MAX;

	pwm_set_dt(pwm_spec, PWM_USEC(PWM_PERIOD), PWM_USEC(scaled_lvl));
}


// ============================= GPIO functions ============================= //

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


// ============================= ADC functions ============================== //

int adc_pin_init(const struct adc_dt_spec *adc_channel, 
			struct adc_channel_ctx *adc_ctx)
{
	int err = 0;
	adc_ctx->adc_channel = adc_channel;
	//init buffer
	adc_ctx->sequence.buffer = &adc_ctx->buf;
	adc_ctx->sequence.buffer_size = sizeof(adc_ctx->buf);

	//configure channel
	err += abs(single_device_init(adc_channel->dev));

	//init channel
	err += abs(adc_channel_setup_dt(adc_ctx->adc_channel));

	//init sequence
	err += abs(adc_sequence_init_dt(adc_ctx->adc_channel, &adc_ctx->sequence));

	return err;
}

int16_t read_adc_digital(struct adc_channel_ctx *adc_ctx)
{
	int err = adc_read(adc_ctx->adc_channel->dev, &adc_ctx->sequence);
	if (err != 0) {
		LOG_ERR("ADC reading failed with code %d", err);
		return INT16_MIN;
	} else {
		return adc_ctx->buf;
	}
}

int32_t read_adc_volt(struct adc_channel_ctx *adc_ctx)
{
	int16_t adc_digital = read_adc_digital(adc_ctx);
	int32_t adc_volt = (int32_t)adc_digital;
	int err = adc_raw_to_millivolts_dt(adc_ctx->adc_channel, &adc_volt);
	if (err != 0) {
		LOG_ERR("ADC reading failed with code %d", err);
		return INT32_MIN;
	} else {
		return adc_volt;
	}
}