#include "lvl_model.h"

static const struct bt_mesh_lvl_srv_handlers lvl_handlers = {
	.set = dimmable_set,
	.get = dimmable_get,
};






//TODO: initialize model
static struct dimmable_ctx myDimmable_ctx[] = {
	{ .srv = BT_MESH_ONOFF_SRV_INIT(&onoff_handlers) },
};