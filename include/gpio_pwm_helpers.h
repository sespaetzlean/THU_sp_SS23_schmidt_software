#ifndef GPIO_PWM_HELPERS_H__
#define GPIO_PWM_HELPERS_H__

#include <zephyr/types.h>
#include <bluetooth/mesh/models.h>

#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/gpio.h>

#ifdef __cplusplus
extern "C" {
#endif


/// @brief checks if the device is ready & outputs info to console
/// @param dev device instance of the pin to control
/// @return Zero on success or (negative) error code otherwise
int single_device_init(const struct device *dev);


/// @brief sets the specific pin to this lvl
/// @param pwm_spec pwm instance of the pin to control
/// @param desired_lvl is mapped between 0 and BT_MESH_LIGHTNESS_MAX
void lc_pwm_output_set(const struct pwm_dt_spec *pwm_spec, 
            const uint16_t desired_lvl);


/// @brief performs necessary steps to register a pin as interrupt 
/// @param spec the pin specification (normally from GPIO_DT_SPEC_GET)
/// @param flags type of interrupt (rising edge, ...)
/// @param callback callback struct 
/// @param handler function to execute on callback
/// @return 0 on success, otherwise sum of error codes
int configure_gpi_interrupt(const struct gpio_dt_spec *spec, 
			gpio_flags_t flags, 
			struct gpio_callback *callback, 
			gpio_callback_handler_t handler);


#ifdef __cplusplus
}
#endif

#endif /* GPIO_PWM_HELPERS_H__ */
