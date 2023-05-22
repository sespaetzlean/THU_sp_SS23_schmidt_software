
/**
 * @file
 * @brief Lc pwm led module
 */

#ifndef LC_PWM_OUTPUT_H__
#define LC_PWM_OUTPUT_H__

#include <zephyr/types.h>
#include <bluetooth/mesh/models.h>
#include <zephyr/drivers/pwm.h>

#ifdef __cplusplus
extern "C" {
#endif


/// @brief checks if the device is ready & outputs info to console
/// @param pwm_dev device instance of the pin to control
/// @return Zero on success or (negative) error code otherwise
int lc_pwm_output_init(const struct device *pwm_dev);

/// @brief sets the specific pin to this lvl
/// @param pwm_spec pwm instance of the pin to control
/// @param desired_lvl is mapped between 0 and BT_MESH_LIGHTNESS_MAX
void lc_pwm_output_set(const struct pwm_dt_spec *pwm_spec, const uint16_t desired_lvl);

#ifdef __cplusplus
}
#endif

#endif /* LC_PWM_OUTPUT_H__ */
