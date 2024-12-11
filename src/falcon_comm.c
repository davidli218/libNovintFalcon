#include <stdio.h>
#include <string.h>

#include "ftd2xx.h"

#include "falcon_check.h"
#include "falcon_comm.h"

static const char* TAG = "FalconComm";

static const char* FALCON_DESCRIPTION = "FALCON HAPTIC";
static const unsigned int MAX_FALCON_DEV = 16;

struct falcon_comm_discover_result {
    unsigned int device_index; /* FTDI device index */
    unsigned int falcon_index; /* Falcon device index */
    char falcon_serial[64];    /* Falcon device serial number */
};

static FalconCommStatus falcon_comm_discover(struct falcon_comm_discover_result* result, int* resultSize) {
    FT_STATUS ftStatus = FT_OK;
    FT_DEVICE_LIST_INFO_NODE* devInfo = NULL;
    DWORD numDevs = 0;

    /* Create DeviceInfoList and retrieve D2XX device count */
    ftStatus = FT_CreateDeviceInfoList(&numDevs);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to create FTD2XX DeviceInfoList", FALCON_COMM_ERR);
    printf("INFO(D): Number of FTD2XX devices: %d\n", numDevs);

    /* Allocate memory for DeviceInfoList */
    devInfo = malloc(sizeof(FT_DEVICE_LIST_INFO_NODE) * numDevs);
    FALCON_CHECK_RETURN(devInfo != NULL, "Failed to allocate memory for FTD2XX DeviceInfoList", FALCON_COMM_ERR);

    /* Retrieve DeviceInfoList */
    ftStatus = FT_GetDeviceInfoList(devInfo, &numDevs);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to get FTD2XX DeviceInfoList", FALCON_COMM_ERR);

    /* Traverse DeviceInfoList to find Falcon devices */
    *resultSize = 0;
    for (int i = 0; i < numDevs; i++) {
        if (!strcmp(devInfo[i].Description, FALCON_DESCRIPTION)) {
            result[*resultSize].device_index = i;
            result[*resultSize].falcon_index = *resultSize;
            strcpy(result[*resultSize].falcon_serial, devInfo[i].SerialNumber);
            *resultSize += 1;
        }
    }

    free(devInfo);

    return FALCON_COMM_OK;
}

FalconCommStatus falcon_comm_get_device_count(uint32_t* deviceCount) {
    FALCON_CHECK_RETURN(deviceCount != NULL, "Invalid param [device_count]", FALCON_COMM_INVALID_PARAM);

    struct falcon_comm_discover_result result[MAX_FALCON_DEV];
    int resultSize = 0;

    const FalconCommStatus status = falcon_comm_discover(result, &resultSize);
    FALCON_CHECK_RETURN(status == FALCON_COMM_OK, "Failed to discover Falcon", FALCON_COMM_ERR);

    *deviceCount = resultSize;

    return FALCON_COMM_OK;
}

FalconCommStatus falcon_comm_open(const uint32_t deviceIndex, FalconCommDevice** fcd) {
    FALCON_CHECK_RETURN(fcd != NULL, "Invalid param [device]", FALCON_COMM_INVALID_PARAM);

    struct falcon_comm_discover_result result[MAX_FALCON_DEV];
    int resultSize = 0;

    /* Retrieve Falcon device list */
    const FalconCommStatus status = falcon_comm_discover(result, &resultSize);
    FALCON_CHECK_RETURN(status == FALCON_COMM_OK, "Failed to discover Falcon", FALCON_COMM_ERR);

    /* Check if Falcon device index exists */
    FALCON_CHECK_RETURN(deviceIndex < resultSize, "Falcon device index does not exist", FALCON_COMM_NOT_FOUND);

    /* Allocate memory for `FalconCommDevice` */
    *fcd = malloc(sizeof(FalconCommDevice));
    FALCON_CHECK_RETURN(*fcd != NULL, "Failed to allocate memory for FalconCommDevice", FALCON_COMM_ERR);

    /* Open Falcon device connection */
    const FT_STATUS ftStatus = FT_OpenEx(
        result[deviceIndex].falcon_serial,FT_OPEN_BY_SERIAL_NUMBER, &(*fcd)->handle
    );
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to connect to Falcon", FALCON_COMM_ERR);

    /* Copy serial number & handle to `FalconCommDevice` */
    strcpy((*fcd)->serial_number, result[deviceIndex].falcon_serial);
    (*fcd)->handle = (*fcd)->handle;

    return FALCON_COMM_OK;
}

FalconCommStatus falcon_comm_close(FalconCommDevice** fcd) {
    FALCON_CHECK_RETURN(fcd != NULL, "Invalid param [device]", FALCON_COMM_INVALID_PARAM);
    FALCON_CHECK_RETURN(*fcd != NULL, "Invalid param [*device]", FALCON_COMM_INVALID_PARAM);

    const FT_STATUS ftStatus = FT_Close((*fcd)->handle);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to close Falcon", FALCON_COMM_ERR);

    free(*fcd);
    *fcd = NULL;

    return FALCON_COMM_OK;
}

FalconCommStatus falcon_comm_read(const FalconCommDevice* fcd, const uint32_t size, uint8_t* rxd) {
    FALCON_CHECK_RETURN(fcd != NULL, "Invalid param [device]", FALCON_COMM_INVALID_PARAM);
    FALCON_CHECK_RETURN(rxd != NULL, "Invalid param [data]", FALCON_COMM_INVALID_PARAM);

    FT_STATUS ftStatus = FT_OK;
    DWORD RxBytes;
    DWORD BytesReceived;

    ftStatus = FT_GetQueueStatus(fcd->handle, &RxBytes);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to retrieve Falcon queue status", FALCON_COMM_ERR);
    FALCON_CHECK_RETURN(RxBytes >= size, "Insufficient data in Falcon for reading", FALCON_COMM_READ_ERR);

    ftStatus = FT_Read(fcd->handle, rxd, size, &BytesReceived);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to read data from Falcon", FALCON_COMM_READ_ERR);
    FALCON_CHECK_RETURN(BytesReceived == size, "Invalid data length read from Falcon", FALCON_COMM_READ_ERR);

    return FALCON_COMM_OK;
}

FalconCommStatus falcon_comm_read_blocking(const FalconCommDevice* fcd, const uint32_t size, uint8_t* rxd) {
    FALCON_CHECK_RETURN(fcd != NULL, "Invalid param [device]", FALCON_COMM_INVALID_PARAM);
    FALCON_CHECK_RETURN(rxd != NULL, "Invalid param [data]", FALCON_COMM_INVALID_PARAM);

    DWORD BytesReceived;

    const FT_STATUS ftStatus = FT_Read(fcd->handle, rxd, size, &BytesReceived);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to read data from Falcon", FALCON_COMM_READ_ERR);
    FALCON_CHECK_RETURN(BytesReceived == size, "Invalid data length read from Falcon", FALCON_COMM_READ_ERR);

    return FALCON_COMM_OK;
}

FalconCommStatus falcon_comm_write(const FalconCommDevice* fcd, const uint8_t* txd, const uint32_t size) {
    FALCON_CHECK_RETURN(fcd != NULL, "Invalid param [device]", FALCON_COMM_INVALID_PARAM);
    FALCON_CHECK_RETURN(txd != NULL, "Invalid param [data]", FALCON_COMM_INVALID_PARAM);

    DWORD BytesWritten;

    const FT_STATUS ftStatus = FT_Write(fcd->handle, (LPDWORD)txd, size, &BytesWritten);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to write data to Falcon", FALCON_COMM_WRITE_ERR);
    FALCON_CHECK_RETURN(BytesWritten == size, "Invalid data length written to Falcon", FALCON_COMM_WRITE_ERR);

    return FALCON_COMM_OK;
}

FalconCommStatus falcon_comm_get_bytes_available(const FalconCommDevice* fcd, uint32_t* bytesAvailable) {
    FALCON_CHECK_RETURN(fcd != NULL, "Invalid param [device]", FALCON_COMM_INVALID_PARAM);
    FALCON_CHECK_RETURN(bytesAvailable != NULL, "Invalid param [bytesAvailable]", FALCON_COMM_INVALID_PARAM);

    DWORD RxBytes;

    const FT_STATUS ftStatus = FT_GetQueueStatus(fcd->handle, &RxBytes);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to retrieve Falcon queue status", FALCON_COMM_ERR);

    *bytesAvailable = RxBytes;

    return FALCON_COMM_OK;
}

FalconCommStatus falcon_comm_has_bytes_available(const FalconCommDevice* fcd, bool* isBytesAvailable) {
    FALCON_CHECK_RETURN(fcd != NULL, "Invalid param [device]", FALCON_COMM_INVALID_PARAM);
    FALCON_CHECK_RETURN(isBytesAvailable != NULL, "Invalid param [isBytesAvailable]", FALCON_COMM_INVALID_PARAM);

    DWORD RxBytes;

    const FT_STATUS ftStatus = FT_GetQueueStatus(fcd->handle, &RxBytes);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to retrieve Falcon queue status", FALCON_COMM_ERR);

    *isBytesAvailable = RxBytes > 0;

    return FALCON_COMM_OK;
}

FalconCommStatus falcon_comm_set_firmware_mode(const FalconCommDevice* fcd) {
    FALCON_CHECK_RETURN(fcd != NULL, "Invalid param [device]", FALCON_COMM_INVALID_PARAM);

    const uint8_t magicCheckArr1Tx[3] = {0x0a, 0x43, 0x0d};
    const uint8_t magicCheckArr1Rx[5] = {0x00, 0x0a, 0x44, 0x2c, 0x0d};
    const uint8_t magicCheckArr2Tx[1] = {0x41};
    const uint8_t magicCheckArr2Rx[2] = {0x13, 0x41};

    FT_STATUS ftStatus = FT_OK;
    FalconCommStatus fcStatus = FALCON_COMM_OK;
    uint8_t rxBuffer[16] = {0};
    int magicCheckResult = 0;

    /* 1. Purge FTDI RX and TX buffers */
    ftStatus = FT_Purge(fcd->handle, FT_PURGE_RX | FT_PURGE_TX);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to purge FTDI RX/TX buffers", FALCON_COMM_ERR);

    /* 2. Set FTDI latency timer to 16ms */
    ftStatus = FT_SetLatencyTimer(fcd->handle, 16);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to set FTDI latency timer", FALCON_COMM_ERR);

    /* 3. Set FTDI to 9600Baud, 8N1, No Flow Control */
    ftStatus = FT_SetBaudRate(fcd->handle, 9600);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to set FTDI baud rate", FALCON_COMM_ERR);
    ftStatus = FT_SetDataCharacteristics(fcd->handle, FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to set FTDI data characteristics", FALCON_COMM_ERR);
    ftStatus = FT_SetFlowControl(fcd->handle, FT_FLOW_NONE, 0, 0);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to set FTDI flow control", FALCON_COMM_ERR);

    /* 4. Set FTDI RTS and DTR to 0, then set DTR to 1 */
    ftStatus = FT_ClrRts(fcd->handle);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to clear FTDI RTS", FALCON_COMM_ERR);
    ftStatus = FT_ClrDtr(fcd->handle);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to clear FTDI DTR", FALCON_COMM_ERR);
    ftStatus = FT_SetDtr(fcd->handle);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to set FTDI DTR", FALCON_COMM_ERR);

    /* 5. Check magic array 1 */
    int i = 0;
    for (i = 0; i < 64; i++) {
        fcStatus = falcon_comm_write(fcd, magicCheckArr1Tx, sizeof(magicCheckArr1Tx));
        FALCON_CHECK_RETURN(fcStatus == FALCON_COMM_OK, "Failed to write magic array to FTDI", FALCON_COMM_ERR);
        fcStatus = falcon_comm_read_blocking(fcd, sizeof(magicCheckArr1Rx), rxBuffer);
        FALCON_CHECK_RETURN(fcStatus == FALCON_COMM_OK, "Failed to read magic array from FTDI", FALCON_COMM_ERR);

        magicCheckResult = memcmp(rxBuffer, magicCheckArr1Rx, sizeof(magicCheckArr1Rx));
        if (magicCheckResult == 0) { break; }
    }
    FALCON_CHECK_RETURN(i < 64, "Failed magic array check 1 with FTDI", FALCON_COMM_ERR);

    /* 6. Set FTDI DTR to 0, then 140000Baud */
    ftStatus = FT_ClrDtr(fcd->handle);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to clear FTDI DTR", FALCON_COMM_ERR);
    ftStatus = FT_SetBaudRate(fcd->handle, 140000);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to set FTDI baud rate", FALCON_COMM_ERR);

    /* 7. Check magic array 2 */
    fcStatus = falcon_comm_write(fcd, magicCheckArr2Tx, sizeof(magicCheckArr2Tx));
    FALCON_CHECK_RETURN(fcStatus == FALCON_COMM_OK, "Failed to write magic array to FTDI", FALCON_COMM_ERR);
    fcStatus = falcon_comm_read_blocking(fcd, sizeof(magicCheckArr2Rx), rxBuffer);
    FALCON_CHECK_RETURN(fcStatus == FALCON_COMM_OK, "Failed to read magic array from FTDI", FALCON_COMM_ERR);

    magicCheckResult = memcmp(rxBuffer, magicCheckArr2Rx, sizeof(magicCheckArr2Rx));
    FALCON_CHECK_RETURN(magicCheckResult == 0, "Failed magic array check 2 with FTDI", FALCON_COMM_ERR);

    return FALCON_COMM_OK;
}

FalconCommStatus falcon_comm_set_normal_mode(const FalconCommDevice* fcd) {
    FALCON_CHECK_RETURN(fcd != NULL, "Invalid param [device]", FALCON_COMM_INVALID_PARAM);

    FT_STATUS ftStatus = FT_OK;

    ftStatus = FT_SetBaudRate(fcd->handle, 1456312);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to set FTDI baud rate", FALCON_COMM_ERR);
    ftStatus = FT_SetLatencyTimer(fcd->handle, 1);
    FALCON_CHECK_RETURN(ftStatus == FT_OK, "Failed to set FTDI latency timer", FALCON_COMM_ERR);

    return FALCON_COMM_OK;
}
