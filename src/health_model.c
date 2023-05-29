#include "health_model.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(att_mod,LOG_LEVEL_DBG);


static bool attention;
struct k_work_delayable attention_blink_work;


void attention_blink(struct k_work *work)
{
	if (attention) {
		LOG_WRN("Attention!");
		k_work_reschedule(&attention_blink_work, K_MSEC(1000));
	} else {
		LOG_INF("Attention turned of");
	}
}

void attention_on(struct bt_mesh_model *mod)
{
	attention = true;
	k_work_reschedule(&attention_blink_work, K_NO_WAIT);
}

void attention_off(struct bt_mesh_model *mod)
{
	/* Will stop rescheduling blink timer */
	attention = false;
}