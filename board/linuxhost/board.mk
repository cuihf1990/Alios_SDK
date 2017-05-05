##
 # Copyright (C) 2016 YunOS Project. All rights reserved.
 #
 # Licensed under the Apache License, Version 2.0 (the "License");
 # you may not use this file except in compliance with the License.
 # You may obtain a copy of the License at
 #
 #   http://www.apache.org/licenses/LICENSE-2.0
 #
 # Unless required by applicable law or agreed to in writing, software
 # distributed under the License is distributed on an "AS IS" BASIS,
 # WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 # See the License for the specific language governing permissions and
 # limitations under the License.
##

IMAGE_FILE := $(YOC_OUT_PATH)/main

L_LDFLAGS := \
    -lpthread \
    -lm \
    -lcrypto

ifneq ($(gcov),n)
L_LDFLAGS += -fprofile-arcs -ftest-coverage
L_LDFLAGS += -DENABLE_GCOV
endif

ifneq ($(CONFIG_YOC_RHINO), y)
L_LDFLAGS += -lrt
endif

ifneq ($(CONFIG_YOC_SSL), y)
L_LDFLAGS += -lssl
endif

ifeq ($(CONFIG_YOC_URADAR_MESH), y)
L_LDFLAGS += -lreadline
endif

ifeq ($(M), 1)
L_LDFLAGS += \
    -Wl,-u,__asan_preinit -lasan
endif

image: $(IMAGE_FILE)

$(IMAGE_FILE): PRIVATE_LDFLAGS:=$(L_LDFLAGS) $(CSP_LDFLAGS)
$(IMAGE_FILE): $(YOC_OUT_TARGET_FILE) $(YOC_L_PRE_LIBS) $(YOC_L_EXCLUDE_LIBS)
	$(CC) -o $@ $^ $(PRIVATE_LDFLAGS)
