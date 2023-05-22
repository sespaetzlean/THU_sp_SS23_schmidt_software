/**
 * @file
 * @brief Model handler
 */

#ifndef HEALTH_MODEL_H__
#define HEALTH_MODEL_H__

#include <zephyr/bluetooth/mesh.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <bluetooth/mesh/models.h>
#include <dk_buttons_and_leds.h>


#ifdef __cplusplus
extern "C" {
#endif


void attention_on(struct bt_mesh_model *mod);

void attention_off(struct bt_mesh_model *mod);

void attention_blink(struct k_work *work);

#ifdef __cplusplus
}
#endif

#endif /* HEALTH_MODEL_H__ */
