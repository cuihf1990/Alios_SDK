NAME := packages-mqtt

$(NAME)_INCLUDES := ../platform/ ./LITE-log/
$(NAME)_SOURCES := LITE-utils/json_parser.c LITE-utils/json_token.c LITE-utils/lite-utils_prog.c LITE-utils/lite-utils_testsuites.c LITE-utils/mem_stats.c LITE-utils/string_utils.c LITE-log/lite-log.c
