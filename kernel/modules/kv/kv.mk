NAME := kv

$(NAME)_SOURCES := kvmgr.c 
$(NAME)_COMPONENTS += log hashtable digest_algorithm

GLOBAL_INCLUDES  += include
