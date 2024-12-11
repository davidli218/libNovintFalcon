#pragma once

#define FALCON_CHECK_RETURN(x, str, ret_val)                               \
    if (!(x)) {                                                            \
        printf("Error %s: %s(%d): %s\n",TAG, __FUNCTION__, __LINE__, str); \
        return (ret_val);                                                  \
    }

#define FALCON_CHECK_LOG(x, str)                                           \
    if (!(x)) {                                                            \
        printf("Error %s: %s(%d): %s\n",TAG, __FUNCTION__, __LINE__, str); \
    }
