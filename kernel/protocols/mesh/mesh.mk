NAME := mesh

GLOBAL_INCLUDES += include

$(NAME)_INCLUDES += include

$(NAME)_SOURCES := src/core/umesh.c \
                   src/core/mesh/mesh_mgmt.c \
                   src/core/mesh/network_mgmt.c \
                   src/core/mesh/link_mgmt.c \
                   src/core/mesh/network_data.c \
                   src/core/mesh/mesh_forwarder.c \
                   src/core/mesh/address_resolver.c \
                   src/core/mesh/lowpan6.c \
                   src/core/mesh/mcast.c \
                   src/core/mesh/address_cache.c \
                   src/core/routing/router_mgr.c \
                   src/core/routing/sid_router.c \
                   src/core/routing/vector_router.c \
                   src/core/routing/ssid_allocator.c \
                   src/core/routing/rsid_allocator.c \
                   src/core/security/dtls.c \
                   src/core/security/keys_mgr.c \
                   src/diags/diags.c \
                   src/ipv6/lwip_adapter.c \
                   src/ipv6/ip6_address.c \
                   src/ipv6/lwip_ip6.c \
                   src/hal/interfaces.c \
                   src/hal/umesh_hal.c \
                   src/utilities/encoding.c \
                   src/utilities/message.c \
                   src/utilities/timer.c \
                   src/utilities/logging.c \
                   src/utilities/mac_whitelist.c \
                   src/utilities/memory.c \
                   src/utilities/configs.c \
                   src/cli/cli.c
