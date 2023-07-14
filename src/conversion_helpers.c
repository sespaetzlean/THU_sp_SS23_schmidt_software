#include "conversion_helpers.h"


uint16_t mesh_level2struct_level(int16_t mesh_level)
{
	return (uint16_t) 32768 + (uint16_t) mesh_level;
}


int16_t struct_level2mesh_level(uint16_t struct_level)
{
	return (int16_t) struct_level - (int16_t) 32768;
}