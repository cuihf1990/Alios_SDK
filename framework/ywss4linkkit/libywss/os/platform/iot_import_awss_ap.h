#ifndef __IMPORT_AWSS_AP_H__
#define __IMPORT_AWSS_AP_H__

#define PRODUCT_IFNAME_LEN      (32)
#define PRODUCT_MAC_LEN         (17)
#define PRODUCT_SSID_LEN        (128)
#define PRODUCT_PWD_LEN         (128)
#define PRODUCT_DEVICEID_LEN    (128)

typedef char *(*GET_IFNAME_FT)(char ifname_str[PRODUCT_IFNAME_LEN + 1]);
typedef int (*GET_AP_INFO_FT)(char ssid_str[PRODUCT_SSID_LEN + 1], \
                        char pwd_str[PRODUCT_PWD_LEN + 1], \
                        char ifname_str[PRODUCT_IFNAME_LEN + 1]);
typedef int (*SET_AP_INFO_FT)(const char *ssid_str, const char *pwd_str, int enable_state, int visible_flag);


typedef struct {
	GET_IFNAME_FT get_awss_ap_bridge_ifname;	//获取配网热点所在bridge的名称
	GET_IFNAME_FT get_adha_port_ifname; 		//获取设备发现热点adha网络接口名称
	GET_IFNAME_FT get_aha_port_ifname;			//获取配网热点aha网络接口名称
	GET_IFNAME_FT get_lan_ifname;				//获取lan测网络接口名称
	GET_AP_INFO_FT get_extranet_ap_info;		//获取上网热点信息
	SET_AP_INFO_FT set_awss_ap_info;			//设置设备发现热点adha和配网热点aha信息
} awssap_ops_t;

#endif  /* __IMPORT_PRODUCT_H__ */
