
/**
 * @file
 * @brief Lc pwm led module
 */

#ifndef LC_PWM_OUTPUT_H__
#define LC_PWM_OUTPUT_H__

#include <zephyr/types.h>
#include <bluetooth/mesh/models.h>

#include <zephyr/drivers/pwm.h>
#include <zephyr/drivers/gpio.h>

#ifdef __cplusplus
extern "C" {
#endif


//define node identifier: for all 4 leds and all 4 buttons
/*
#define LED0_NODE    DT_ALIAS(led0)
#define LED1_NODE    DT_ALIAS(led1)
#define LED2_NODE    DT_ALIAS(led2)
#define LED3_NODE    DT_ALIAS(led3)

#define SW0_NODE     DT_ALIAS(sw0)
#define SW1_NODE     DT_ALIAS(sw1)
#define SW2_NODE     DT_ALIAS(sw2)
#define SW3_NODE     DT_ALIAS(sw3)
*/
/* This could be done via several ways:
- DT_PATH()
- DT_ALIAS()
- DT_NODELABEL()
- DT_INST()
*/


//retrieve device pointer, pin number & configuration flags:
/*
static const struct gpio_dt_spec led0_spec = GPIO_DT_SPEC_GET(LED0_NODE, gpios); //gpios is the property name!
static const struct gpio_dt_spec led1_spec = GPIO_DT_SPEC_GET(LED1_NODE, gpios);
static const struct gpio_dt_spec led2_spec = GPIO_DT_SPEC_GET(LED2_NODE, gpios);
static const struct gpio_dt_spec led3_spec = GPIO_DT_SPEC_GET(LED3_NODE, gpios);

static const struct gpio_dt_spec sw0_spec = GPIO_DT_SPEC_GET(SW0_NODE, gpios);
static const struct gpio_dt_spec sw1_spec = GPIO_DT_SPEC_GET(SW1_NODE, gpios);
static const struct gpio_dt_spec sw2_spec = GPIO_DT_SPEC_GET(SW2_NODE, gpios);
static const struct gpio_dt_spec sw3_spec = GPIO_DT_SPEC_GET(SW3_NODE, gpios);
*/

//make sure the device is ready:
/*
if (!device_is_ready(led.port)) {
    !
}
*/

// === configure GPIO === //
/*
int err;
//configure as output
err = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
//configure as input
err = gpio_pin_configure_dt(&led, GPIO_INPUT);
if (err < 0) {
    !
}
*/

// === interact with GPIO === //
/*
//read from gpio 
!(via polling)
bool val = gpio_pin_get_dt(&button);
//write to gpio
gpio_pin_set_dt(&led,val);
*/

// === interact via Interrupts === //
/*
int err;
* 1. configure interrupt
err = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
* 2. define callback function
void button_pressed_cb(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    TODO: implement necessary todos
    gpio_pin_toggle_dt(&led);
}
* 3. define variable of type struct gpio_callback
static struct gpio_callback button_cb_data;
* 4. initialize callback variable
gpio_init_callback(&button_cb_data, button_pressed_cb, BIT(button.pin));
* 5. add callback
gpio_add_callback(button.port, &button_cb_data);

*/



/// @brief checks if the device is ready & outputs info to console
/// @param pwm_dev device instance of the pin to control
/// @return Zero on success or (negative) error code otherwise
int single_device_init(const struct device *pwm_dev);


/// @brief sets the specific pin to this lvl
/// @param pwm_spec pwm instance of the pin to control
/// @param desired_lvl is mapped between 0 and BT_MESH_LIGHTNESS_MAX
void lc_pwm_output_set(const struct pwm_dt_spec *pwm_spec, const uint16_t desired_lvl);



int configure_gpi_interrupt(const struct gpio_dt_spec *spec, 
			gpio_flags_t flags, 
			struct gpio_callback *callback, 
			gpio_callback_handler_t handler);


#ifdef __cplusplus
}
#endif

#endif /* LC_PWM_OUTPUT_H__ */
