#include "conversion_helpers.h"

/// @brief convert a signed int to an unsigned int 
/// @param inputLevel the signed int
/// @return the mapped unsigned int
uint16_t input_level2bt_level(int16_t inputLevel)
{
	return (uint16_t) 32768 + (uint16_t) inputLevel;
}

/// @brief convert an unsigned int to a signed int 
/// @param bt_level the unsigned int
/// @return the mapped signed int
int16_t bt_level2input_level(uint16_t bt_level)
{
	return (int16_t) bt_level - (int16_t) 32768;
}