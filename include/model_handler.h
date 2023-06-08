#ifndef MODEL_HANDLER_H__
#define MODEL_HANDLER_H__

#include <zephyr/bluetooth/mesh.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>


#include "health_model.h"
#include "relais_model.h"
#include "lvl_model.h"
#include "lightness_model.h"
// =====================
#include "relais_cli_mod.h"
#include "lvl_cli_mod.h"

#include"gpio_pwm_helpers.h"					//for in & outut (gpio)



#ifdef __cplusplus
extern "C" {
#endif

/// @brief shall be called when initializing bluetooth stack
/// (normally via bt_mesh_init)
const struct bt_mesh_comp *model_handler_init(void);

#ifdef __cplusplus
}
#endif

#endif /* MODEL_HANDLER_H__ */
