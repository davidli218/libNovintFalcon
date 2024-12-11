#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "ftd2xx.h"

/**
* @brief The status of the Falcon communication operation
*/
typedef enum {
    FALCON_COMM_OK,            /**< Operation successful */
    FALCON_COMM_ERR,           /**< Operation failed */
    FALCON_COMM_INVALID_PARAM, /**< Invalid parameter */
    FALCON_COMM_NOT_FOUND,     /**< Falcon device not found */
    FALCON_COMM_READ_ERR,      /**< Read error */
    FALCON_COMM_WRITE_ERR,     /**< Write error */
} FalconCommStatus;

/**
* @brief The Falcon communication device
*/
typedef struct {
    char serial_number[64]; /**< The Falcon device serial number */
    FT_HANDLE handle;       /**< The Falcon device FTD2XX handle */
} FalconCommDevice;

/**
 * @brief Retrieve the number of Falcon devices connected to the system
 *
 * @param[out] deviceCount Pointer to a variable that will hold the number of connected Falcon devices.
 *
 * @return The status of the operation.
 */
FalconCommStatus falcon_comm_get_device_count(uint32_t* deviceCount);

/**
 * @brief Open a connection to a specific Falcon device
 *
 * @param[in]  deviceIndex Index of the Falcon device to open.
 * @param[out] fcd         Pointer to the handle of the FalconCommDevice.
 *
 * @return The status of the operation.
 */
FalconCommStatus falcon_comm_open(uint32_t deviceIndex, FalconCommDevice** fcd);

/**
 * @brief Close the connection to a Falcon device
 *
 * @param[in,out] fcd Pointer to the handle of the FalconCommDevice.
 *
 * @return The status of the operation.
 */
FalconCommStatus falcon_comm_close(FalconCommDevice** fcd);

/**
 * @brief Read data from a Falcon device in non-blocking mode
 *
 * @param[in]  fcd  Handle of the FalconCommDevice.
 * @param[in]  size Number of bytes to read from the device.
 * @param[out] rxd  Pointer to the buffer where the received data will be stored.
 *
 * @return The status of the operation.
 */
FalconCommStatus falcon_comm_read(const FalconCommDevice* fcd, uint32_t size, uint8_t* rxd);

/**
 * @brief Read data from a Falcon device in blocking mode
 *
 * @param[in]  fcd  Handle of the FalconCommDevice.
 * @param[in]  size Number of bytes to read from the device.
 * @param[out] rxd  Pointer to the buffer where the received data will be stored.
 *
 * @return The status of the operation.
 */
FalconCommStatus falcon_comm_read_blocking(const FalconCommDevice* fcd, uint32_t size, uint8_t* rxd);

/**
 * @brief Write data to a Falcon device
 *
 * @param[in] fcd  Handle of the FalconCommDevice.
 * @param[in] txd  Pointer to the buffer containing the data to be written.
 * @param[in] size Number of bytes to write to the device.
 *
 * @return The status of the operation.
 */
FalconCommStatus falcon_comm_write(const FalconCommDevice* fcd, const uint8_t* txd, uint32_t size);

/**
 * @brief Retrieve the number of bytes available to read from a Falcon device
 *
 * @param[in]  fcd            Handle of the FalconCommDevice.
 * @param[out] bytesAvailable Pointer to a variable where the result will be stored.
 *
 * @return The status of the operation.
 */
FalconCommStatus falcon_comm_get_bytes_available(const FalconCommDevice* fcd, uint32_t* bytesAvailable);

/**
 * @brief Check if there are bytes available to read from a Falcon device
 *
 * @param[in]  fcd              Handle of the FalconCommDevice.
 * @param[out] isBytesAvailable Pointer to a variable that will be set to `true` if data is available.
 *
 * @return The status of the operation.
 */
FalconCommStatus falcon_comm_has_bytes_available(const FalconCommDevice* fcd, bool* isBytesAvailable);

/**
 * @brief Set the Falcon device to firmware mode
 *
 * @param[in] fcd Handle of the FalconCommDevice.
 *
 * @return The status of the operation.
 */
FalconCommStatus falcon_comm_set_firmware_mode(const FalconCommDevice* fcd);

/**
 * @brief Set the Falcon device to normal mode
 *
 * @param[in] fcd Handle of the FalconCommDevice.
 *
 * @return The status of the operation.
 */
FalconCommStatus falcon_comm_set_normal_mode(const FalconCommDevice* fcd);
