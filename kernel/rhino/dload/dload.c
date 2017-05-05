/****************************************************************************
 *
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 ****************************************************************************/

 /**
 * @file
 * @brief      dynamic loader
 * @details    ymod file dynamic loader
 * @author     zhifang Xiu
 * @date       2016-12-06
 * @version    0.1
 */

#include <stdlib.h>
#include <string.h>
#include "ymod.h"
#include "dload.h"
#include "dload_port.h"

#if (YUNOS_CONFIG_DLOAD_SUPPORT > 0)
/****************************************************************************
 * Definitions
 ****************************************************************************/

/* module management information */
typedef struct mod_info {
    uint8_t  valid;            /* whether this item is valid. 0 - not valid, 1 - valid */
    uint8_t  rsv[3];
    int32_t  image_fd;         /* module image file descriptor, negative is invalid */
    uint32_t data_ram_base;    /* allocated ram space base for data&bss section */
    uint32_t data_size;        /* size of data&bss section */
    int32_t  map_inst;         /* mmu map instance for this module */
}mod_info_t;

static mod_info_t g_mod_info[YUNOS_CONFIG_DLOAD_MOD_NUM];


/****************************************************************************
 * Functions
 ****************************************************************************/

static uint8_t search_free_item(void)
{
    uint8_t idx;
    for(idx = 0u; idx < YUNOS_CONFIG_DLOAD_MOD_NUM; idx++) {
        if(0u == g_mod_info[idx].valid){
            return idx;
        }
    }
    return 0xFFu;
}

static uint8_t search_item(int32_t image_fd)
{
    uint8_t idx;
    for(idx = 0u; idx < YUNOS_CONFIG_DLOAD_MOD_NUM;idx++) {
        if(1u == g_mod_info[idx].valid
           && g_mod_info[idx].image_fd == image_fd){
            return idx;
        }
    }
    return 0xFFu;
}

static void free_item(uint8_t item)
{
    if(item >= YUNOS_CONFIG_DLOAD_MOD_NUM) {
        return;
    }
    memset((void *)&g_mod_info[item],0x00,sizeof(mod_info_t));
    g_mod_info[item].image_fd = -1;
}

/**
 *  dynamic loader initialization
 *
 * @return       none
 * @note         must only init once at system start
 */
void ymod_init(void)
{
    uint16_t idx;

    memset( (void *)g_mod_info, 0x00, YUNOS_CONFIG_DLOAD_MOD_NUM*sizeof(mod_info_t));
    for(idx = 0;idx < YUNOS_CONFIG_DLOAD_MOD_NUM;idx++) {
        g_mod_info[idx].image_fd = -1;
    }

    cmmu_enable(1);
}

/**
 *  load specified image file
 *
 * @param[in]    image_fd   image file descriptor to be loaded
 * @return       0-success , other-error code. see definition of dload_err_t
 */
int32_t ymod_load(int32_t image_fd)
{
    struct ymod_hdr *header = NULL;
    uint32_t image_base;
    uint32_t image_data_base;
    uint32_t data_ram_base = 0;
    int32_t  ret;
    uint16_t crc;
    uint8_t  item = 0xffu;
    int32_t  map_inst = -1;
#if defined(YUNOS_CONFIG_DLOAD_API_TABLE)
    struct api_table_s **apiptr;
#endif

    item = search_item(image_fd);
    if(item < YUNOS_CONFIG_DLOAD_MOD_NUM) {
        LOG_E("already loaded fd:%x",image_fd);
        return -E_DLD_ALREADY_LOADED;
    }

    /* get management item for this module */
    item = search_free_item();
    if(item >= YUNOS_CONFIG_DLOAD_MOD_NUM) {
        LOG_E("no item:%x",image_fd);
        return -E_DLD_OVERRANGE;
    }

    /* get image data address */
    image_base = nsfs_get_image_base(image_fd);
    if(0xFFFFFFFFu == image_base) {
        LOG_E("image addr err:%x",image_fd);
        ret = -E_DLD_INV_VAL;
        goto fail;
    }

    header = (struct ymod_hdr *)image_base;

    /* check file magic */
    if (strncmp(header->magic, YMOD_MAGIC, 2)) {
        LOG_E("magic err:%c-%c",header->magic[0],header->magic[1]);
        ret = -E_DLD_INV_FMT;
        goto fail;
    }
    LOG_D("image info: %x\n"
          "%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n%x\n",
          image_base,
          header->crc,header->file_ver,header->mod_ver,
          header->uuid0,header->uuid1,
          header->text_len,header->data_len,header->bss_len,
          header->init_base,header->deinit_base);

    crc = crc16((unsigned char const *)(image_base + 4), sizeof(struct ymod_hdr) + header->text_len + header->data_len - 4);
    if (crc != header->crc) {
        LOG_E("crc err:%x-%x",header->crc,crc);
        ret = -E_DLD_CRC_CHK;
        goto fail;
    }

    /* allocate ram for .data & .bss section */
    data_ram_base = (uint32_t)dload_allocate_ram(header->data_len + header->bss_len);
    if(data_ram_base == (uint32_t)NULL) {
        LOG_E("no ram:%x",image_fd);
        ret = -E_DLD_NO_MEM;
        goto fail;
    }

    /* copy .data to run space  */
    image_data_base = image_base + sizeof(struct ymod_hdr) + header->text_len;
    memcpy((void*)data_ram_base,(const void *)image_data_base,header->data_len);

    /* set .bss section to zero  */
    memset((void*)(data_ram_base+header->data_len),0x00,header->bss_len);

    /* now we should flush cache */
    //yunos_csp_flush_cache_range(data_ram_base, data_ram_base + header->data_len + header->bss_len);

    /* data sectin mapping */
    map_inst = cmmu_map(image_data_base,data_ram_base,header->data_len + header->bss_len);
    if(map_inst < 0) {
        LOG_E("no cmmu:%x",image_fd);
        ret = -E_DLD_CMMU;
        goto fail;
    }

#if defined(YUNOS_CONFIG_DLOAD_API_TABLE)
    /* amend api base.  api address pointer is in the beginning  of .data  */
    LOG_D("set api base-%x",&g_api_table);
    apiptr = (struct api_table_s **)data_ram_base;
    *apiptr = &g_api_table;
    LOG_D("api base-%x",*apiptr);
#endif

    /*  execute module init function */
    LOG_D("exec module_init-%x",header->init_base);
    module_entry_t module_init = (module_entry_t)(image_base + sizeof(struct ymod_hdr) + header->init_base);
    module_init();

    g_mod_info[item].valid = 1;
    g_mod_info[item].image_fd = image_fd;
    g_mod_info[item].map_inst = map_inst;
    g_mod_info[item].data_ram_base = data_ram_base;
    g_mod_info[item].data_size = header->data_len + header->bss_len;

    return 0;

fail:
    if(item < YUNOS_CONFIG_DLOAD_MOD_NUM ) {
        free_item(item);
    }
    if(data_ram_base != (uint32_t)NULL ) {
        dload_free_ram((void*)data_ram_base,header->data_len + header->bss_len);
    }
    return ret;
}

/**
 *  unload specified image file
 *
 * @param[in]    image_fd   image file descriptor to be unloaded
 * @return       0-success , other-error code. see definition of dload_err_t
 * @note         this function do not delete the image file,only stop running
 */
int32_t ymod_unload(int32_t image_fd)
{
    uint32_t image_base;
    struct ymod_hdr *header;
    uint8_t  item = 0xFFu;

    /* get item of this module */
    item = search_item(image_fd);
    if(item >= YUNOS_CONFIG_DLOAD_MOD_NUM) {
        LOG_E("no fd:%x",image_fd);
        return -E_DLD_NOT_EXIST;
    }

    /* get image address */
    image_base = nsfs_get_image_base(image_fd);
    if(0xFFFFFFFFu == image_base) {
        return -E_DLD_INV_VAL;
    }

    header = ((struct ymod_hdr *)image_base);

    /* execute module deinit function */
    module_entry_t module_deinit = (module_entry_t)(image_base + sizeof(struct ymod_hdr) + header->deinit_base);
    module_deinit();

    /* free allocated ram size for this module */
    dload_free_ram((void*)g_mod_info[item].data_ram_base,g_mod_info[item].data_size);

    /* data sectin unmapping */
    cmmu_unmap(g_mod_info[item].map_inst);

    free_item(item);

    return 0;
}

/**
 *  get data size(.data&.bss section) from image file
 *
 * @param[in]    image_fd   image file descriptor
 * @return       data size in bytes. negative indicates error code, see definition of dload_err_t
 */
int32_t ymod_get_data_size(int32_t image_fd)
{
    struct ymod_hdr *header;
    uint32_t image_base;
    /* get image address */
    image_base = nsfs_get_image_base(image_fd);
    if(0xFFFFFFFFu == image_base) {
        return -E_DLD_NOT_EXIST;
    }

    header = ((struct ymod_hdr *)image_base);
    if (strncmp(header->magic, YMOD_MAGIC, 2)) {
        LOG_E("magic check error: %c-%c\n",header->magic[0],header->magic[1]);
        return -E_DLD_INV_FMT;
    }

    return (header->data_len + header->bss_len);
}

static void show_ymod_info(uint8_t mod_idx)
{
    struct ymod_hdr *header;
    uint32_t image_base;
    image_base = nsfs_get_image_base(g_mod_info[mod_idx].image_fd);
    if(0xFFFFFFFFu == image_base) {
        return;
    }
    header = ((struct ymod_hdr *)image_base);

    PRINT("---------- mod_idx: %d ----------\n",mod_idx);
    PRINT("image fd:         %d\n",g_mod_info[mod_idx].image_fd);\
    PRINT("image base:       0x%x\n",nsfs_get_image_base(g_mod_info[mod_idx].image_fd));
    PRINT("data flash base:  0x%x\n",image_base + sizeof(struct ymod_hdr) + header->text_len);
    PRINT("data ram base:    0x%x\n",g_mod_info[mod_idx].data_ram_base);
    PRINT("data size:        0x%x\n",g_mod_info[mod_idx].data_size);
    PRINT("map inst:         %d\n",g_mod_info[mod_idx].map_inst);

    PRINT("image info:\n");
    PRINT("    crc:          0x%x\n",header->crc);
    PRINT("    file_ver:     0x%x\n",header->file_ver);
    PRINT("    mod_ver:      0x%x\n",header->mod_ver);
    PRINT("    uuid0:        0x%x\n",header->uuid0);
    PRINT("    uuid1:        0x%x\n",header->uuid1);
    PRINT("    text_len:     0x%x\n",header->text_len);
    PRINT("    data_len:     0x%x\n",header->data_len);
    PRINT("    bss_len:      0x%x\n",header->bss_len);
    PRINT("    init_base:    0x%x\n",header->init_base);
    PRINT("    deinit_base:  0x%x\n",header->deinit_base);
}

/**
 *  show running module information
 *
 * @param    none
 * @return   none
 */
void ymod_show_status(void)
{
    uint8_t idx;
    PRINT("\n======= dynamic module infomation begin =======\n");
    for(idx = 0u; idx < YUNOS_CONFIG_DLOAD_MOD_NUM;idx++) {
        if(1u == g_mod_info[idx].valid){
            show_ymod_info(idx);
        }
    }
    PRINT("======= dynamic module infomation finish =======\n\n");
}


#endif /*YUNOS_CONFIG_DLOAD_SUPPORT*/
