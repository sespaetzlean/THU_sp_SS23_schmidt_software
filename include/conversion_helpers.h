/**
 * @file
 * @brief Model handler
 */

#ifndef CONVERSION_HELPERS_H__
#define CONVERSION_HELPERS_H__

#include <zephyr/bluetooth/mesh.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/mesh/proxy.h>
#include <bluetooth/mesh/models.h>


#ifdef __cplusplus
extern "C" {
#endif

/// @brief convert a signed int to an unsigned int 
/// @param inputLevel the signed int
/// @return the mapped unsigned int
uint16_t input_level2bt_level(int16_t inputLevel);


/// @brief convert an unsigned int to a signed int 
/// @param bt_level the unsigned int
/// @return the mapped signed int
int16_t bt_level2input_level(uint16_t bt_level);

#ifdef __cplusplus
}
#endif

#endif /* CONVERSION_HELPERS_H__ */
