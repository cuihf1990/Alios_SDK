TFS - Trusted Foundation Service
==============

* [1.概述] (#1-概述)
* [2.架构] (#2-架构)
* [3.代码目录说明](#3-代码目录说明)
* [4.移植指南](#4-移植指南)
* [5.接口说明](#5-接口说明)
* [6.功能展示](#6-功能展示)
* [7.单元测试](#7-单元测试)
* [8.Footprint和接口性能](#8-Footprint和接口性能)

1-概述
-------
---
TFS是设备端的安全组件, 可以支持ID2、Pay、Fingerprint等安全服务。

2-架构
--------
---

![image](http://gitlab.alibaba-inc.com/zuojun.zj/tfs20/raw/master/docs/pics/arch3.png)

3-代码目录说明
---------
---
├── `build`  TFS SDK 编译脚本

├── `demo`  TFS的使用Demo

├── `external`  TFS SDK 依赖的其他库，如单元测试框架库，模拟方式下需要的openssl库等

├── `include`  TFS API的头文件

├── `lib`  厂商提供的TFS HAL 库，以.a形式存在

├── `platform`  平台抽象层，根据具体平台进行修改

├── `prebuilts`  TFS SDK 编译所需的交叉编译工具

├── `src`  TFS 核心代码和平台相关代码

└── `uttest`  TFS 单元测试

4-移植指南
-------
---
* **4.1 接入新平台**
 + 在目录platform/下添加平台目录，如freertos，ucos，threadx，mico等。
 + 参照linux平台目录，在平台目录（如freertos，ucos，threadx，mico等）下实现平台相关接口。
 + 在目录prebuilts下添加平台和/或MCU目录，如esp8266使用freertos系统，建立目录freertos/esp8266/，并将esp8266的编译链放到该目录下。
 + 在编译脚本中添加相应的平台和/或MCU的处理逻辑，如esp8266的处理逻辑如下：  
    ```
    processESP8266() {
            if [ ! -d "/opt/xtensa-lx106-elf" ];then
                pushd ../prebuilts/freertos/esp8266 > /dev/null
                sudo tar -jxvf xtensa-lx106-elf-ccf1cfd2.tar.bz2 -C /opt/
                popd > /dev/null
            fi
    
            export PATH=$PATH:/opt/xtensa-lx106-elf/bin
            CC=xtensa-lx106-elf-gcc
            AR=xtensa-lx106-elf-ar
    }
    # cc and ar configuration for differ mcu
    case $mcu in
    esp8266)
            processESP8266
            ;;
    *)
            ;;
    esac
    ```
 + 测试TFS功能。在编译TFS库时，选择lib-debug会将一段测试代码打包到libtfs.a中，在系统的测试代码中调用tfs_test接口即可测试移植的TFS代码的功能。编译正式的libtfs.a时，要选择lib-release，这时lib中就不会包含测试代码。  

* **4.2 平台相关接口** 详见[src/platform/pal.h](http://gitlab.alibaba-inc.com/zuojun.zj/tfs20/blob/master/src/platform/pal.h)
 + 内存相关接口  
   `void *pal_memory_malloc(int size);` 内存分配  
   `void pal_memory_free(void *ptr);` 内存释放
 + 随机数生成接口  
   `int pal_get_random();` 生成随机数
 + 网络相关接口  
   `int pal_network_create(const char *server, int port);` 创建网络套接字  
   `int pal_network_send(int sockFd, const char *buf, int len);` 发送网络数据包  
   `int pal_network_recv(int sockfd, char *buf, int *len);` 接收网络数据包  
   `int pal_network_close(int fd);` 关闭网络套接字
 + BASE64相关接口  
   `void pal_base64_encode(const unsigned char *src, int len, unsigned char *dst, int *out_len);` 对传入的数据进行BASE64编码
 + MD5相关接口  
   `void pal_md5_sum(const uint8_t *addr, const int len, uint8_t *mac);` 计算传入数据的MD5值
 + JSON相关接口  
   `int pal_json_get_string_value(char *json_str, const char **tokens, int tokens_size, char *value);` 获取JSON串中的字符串数据  
   `int pal_json_get_number_value(char *json_str, const char **tokens, int tokens_size, int *value);` 获取JSON串中的整型数据
 + 产品相关接口（注：只实现与本产品相关的接口）  
   `const char *pal_get_product_name();` 获取产品名字  
   `const char *pal_get_imei();` 获取IMEI  
   `const char *pal_get_hardware_id();` 获取设备ID  
   `const char *pal_get_mac();` 获取MAC地址  
   `const char *pal_get_bt_mac();` 获取蓝牙的MAC地址  
   `const char *pal_get_build_time();` 获取编译的时间  
   `const char *pal_get_os_version();` 获取系统的版本  
   `const char *pal_get_dm_pixels();` 获取分辨率  
   `const char *pal_get_dm_dpi();` 获取DPI（每英寸像素）  
   `const char *pal_get_cpu_info();` 获取CPU信息  
   `const char *pal_get_storage_total();` 获取存储总量  
   `const char *pal_get_camera_resolution();` 获取相机分辨率  
 + 存储相关接口  
   `int pal_save_info(const char *key, char *value);` 保存key，value信息  
   `int pal_get_info(const char *key, char *value);` 获取key，value信息  

5-接口说明
--------
---
* **5.1 ID2相关接口**
 + `tfs_get_ID2`  获取设备的ID2
 + `tfs_id2_sign`  使用ID2的私钥对数据签名
 + `tfs_id2_verify`  使用ID2的公钥验签数据的签名
 + `tfs_id2_encrypt`  使用ID2的公钥加密数据
 + `tfs_id2_decrypt`  使用ID2的私钥解密数据
 + `tfs_get_auth_code`  获取设备认证码：mode~sid~seed~signature(id2~sid~seed)
 + `tfs_id2_get_auth_code`  获取设备认证码：model~sid~timestamp~signature(id2~sid~timestamp)
 + `tfs_id2_get_digest_auth_code`  获取数据摘要的认证码：model~timestamp~signature(id2~digest~timestamp)
 + `tfs_activate_device`  设备激活接口
 + `tfs_is_device_activated`  获取设备激活状态
* **5.2 AES加解密接口**
 + `tfs_aes128_cbc_enc` AES128 (CBC模式)数据加密
 + `tfs_aes128_cbc_dec` AES128 (CBC模式)数据解密

6-TFS Demo
-------
---
TFS Demo为TFS接口使用Sample，包括了对TFS 所有接口的调用实现。
* **6.1 TFS Demo编译**
 + 进入build目录；
 + 运行编译脚本build.sh；
 + 选择linux 平台，选择3DES或者RSA（该版本中两种方式均已经支持），选择编译目标为demo。
* **6.2 TFS Demo运行**  
  运行out目录下的tfs_demo,即可以看到TFS 所有接口的调用和调用结果。

7-单元测试
-------
---
* **7.1 目标**
 + 当TFS添加新功能时，能快速的验证新接口的功能完备性和鲁棒性。
 + 当移植到新平台时，能验证TFS接口在新的平台的功能完备性和鲁棒性。
* **7.2 平台相关**
 + 目前linux平台的TFS单元测试可以正常运行。
 + 由于使用的是cpputest测试框架，该框架是C++实现的框架，在不支持C++的IOT平台上无法正常运行，后续我们会更新测试框架，保证各个平台上TFS单元测试的正常运行。
* **7.3 如何编译**  
  同[TFS Demo编译](#6-功能展示)，编译目标选择为uttest。

8-Footprint和接口性能
---------
---
* **8.1 测试环境**
 + 系统：Ubuntu 12.04.5 LTS 64位
 + 配置：Intel(R) Xeon(R) CPU E5620 主频 2.40GHz 16核
 + 加解密接口测试数据为100字节，测试1000次。
* **8.2 Footprint**
 + 加密方式为RSA：`15148 Bytes`
 + 加密方式为3DES：`16562 Bytes`
* **8.3 TFS接口性能**
 + `tfs_get_ID2` 3us
 + `tfs_id2_sign(3DES)` 1583us
 + `tfs_id2_verify(3DES)` 1588us
 + `tfs_id2_encrypt(3DES)` 6901us
 + `tfs_id2_decrypt(3DES)` 6885us
 + `tfs_id2_sign(RSA)` 1441us
 + `tfs_id2_verify(RSA)` 39us
 + `tfs_id2_encrypt(RSA)` 38us
 + `tfs_id2_decrypt(RSA)` 1435us