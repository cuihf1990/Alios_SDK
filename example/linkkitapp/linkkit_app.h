#ifndef __LINKKITAPP_H__
#define __LINKKITAPP_H__

int linkkit_app(void);

#define EVENT_ERROR_IDENTIFIER                 "Error"
#define EVENT_ERROR_OUTPUT_INFO_IDENTIFIER     "ErrorCode"
#define EVENT_CUSTOM_IDENTIFIER                "Custom"

/* user sample context struct. */
typedef struct _sample_context {
    const void* thing;

    int cloud_connected;
    int thing_enabled;

    int service_custom_input_transparency;
    int service_custom_output_contrastratio;
#ifdef SERVICE_OTA_ENABLED
    char ota_buffer[OTA_BUFFER_SIZE];
#endif /* SERVICE_OTA_ENABLED */
} sample_context_t;

#endif
