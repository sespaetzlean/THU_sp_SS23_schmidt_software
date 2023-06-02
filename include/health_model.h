/**
 * @file
 * @brief Model handler
 */

#ifndef HEALTH_MODEL_H__
#define HEALTH_MODEL_H__

#include <zephyr/bluetooth/mesh.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>

#include "lc_pwm_output.h"


#ifdef __cplusplus
extern "C" {
#endif


/// @brief function to execute when node shall attract attention
/// @param mod 
void attention_on(struct bt_mesh_model *mod);

/// @brief function to stop attracting attention
/// @param mod 
void attention_off(struct bt_mesh_model *mod);

/// @brief work struct that executes tasks to attract attention
/// @param work 
void attention_blink(struct k_work *work);


/// @brief shall be called before attention is used
/// @return 0 on success
int attention_init();

#ifdef __cplusplus
}
#endif

#endif /* HEALTH_MODEL_H__ */
