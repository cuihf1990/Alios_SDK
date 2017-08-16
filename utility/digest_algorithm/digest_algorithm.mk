NAME := share_digest_algorithm

$(NAME)_SOURCES := digest_algorithm.c CheckSumUtils.c crc.c md5.c
GLOBAL_INCLUDES += . 
