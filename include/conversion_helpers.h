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
uint16_t mesh_level2struct_level(int16_t mesh_level);


/// @brief convert an unsigned int to a signed int 
/// @param struct_level the unsigned int
/// @return the mapped signed int
int16_t struct_level2mesh_level(uint16_t struct_level);

#ifdef __cplusplus
}
#endif

#endif /* CONVERSION_HELPERS_H__ */
