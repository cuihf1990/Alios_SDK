#ifndef _HAL_H_
#define _HAL_H_

#include "type.h"

#define TFS_SUCCESS             0X00

#define TFS_NO_MEMORY           0XC0
#define TFS_INVALID_LENGTH      0XC1
#define TFS_SECURITY_VIOLATION  0XC2
#define TFS_OPERATION_VIOLATION 0XC3
#define TFS_UNSUPPORTED_FEATURE 0XC4
#define TFS_STRUCTURE_INCORRECT 0XC5
#define TFS_SPACE_NOT_ENOUGH    0XC6
#define TFS_INVALID_ARGUMENT    0XC7
#define TFS_FILE_NOT_FOUND      0XC8
#define TFS_RECORD_NOT_FOUND    0XC9
#define TFS_OPERATION_FAILED    0XCA
#define TFS_INVALID_SIGNATURE   0XCB
#define TFS_CHECKSUM_ERROR      0XCC
#define TFS_INVALID_PACKAGE     0XCB

#ifdef __cplusplus
extern "C" {
#endif

int open_session(void **handle);
int invoke_command(void *handle, uint32_t cmd,
                   void *intput, uint32_t in_len,
                   void *output, uint32_t *out_len);
int close_session(void *handle);

#ifdef __cplusplus
}
#endif

#endif
