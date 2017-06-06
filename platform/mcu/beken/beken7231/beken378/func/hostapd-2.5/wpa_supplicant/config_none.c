/*
 * WPA Supplicant / Configuration backend: empty starting point
 * Copyright (c) 2003-2005, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 *
 * This file implements dummy example of a configuration backend. None of the
 * functions are actually implemented so this can be used as a simple
 * compilation test or a starting point for a new configuration backend.
 */

#include "includes.h"

#include "common.h"
#include "config.h"
#include "base64.h"
#if CFG_MODE_SWITCH
#include "param_config.h"
#endif
#include "uart_pub.h"

static int wpa_config_validate_network(struct wpa_ssid *ssid, int line)
{
	int errors = 0;

	if ((ssid->group_cipher & WPA_CIPHER_CCMP) &&
	    !(ssid->pairwise_cipher & WPA_CIPHER_CCMP) &&
	    !(ssid->pairwise_cipher & WPA_CIPHER_NONE)) {
		/* Group cipher cannot be stronger than the pairwise cipher. */
		wpa_printf(MSG_DEBUG, "Line %d: removed CCMP from group cipher"
			   " list since it was not allowed for pairwise "
			   "cipher", line);
		ssid->group_cipher &= ~WPA_CIPHER_CCMP;
	}

	if (ssid->mode == WPAS_MODE_MESH &&
	    (ssid->key_mgmt != WPA_KEY_MGMT_NONE &&
	    ssid->key_mgmt != WPA_KEY_MGMT_SAE)) {
		wpa_printf(MSG_ERROR,
			   "Line %d: key_mgmt for mesh network should be open or SAE",
			   line);
		errors++;
	}

	return errors;
}

#if CFG_MODE_SWITCH
static char *str_format(char *buf, char *str, int len)
{
	char *p = buf;

	os_memset(p, 0, 20);
	*p++ = '"';
	os_memcpy(p, str, len);
	p += len;
	*p = '"';

	return buf;
}
#endif

int wpa_config_set_none(struct wpa_ssid *ssid)
{
	int ret = 0;
	
	if(wpa_config_set(ssid, "key_mgmt", "NONE", 0) < 0){
		ret++;
	}
	if(wpa_config_set(ssid, "mem_only_psk", "0", 0) < 0){
		ret++;
	}

	if(!ret){
		g_sta_param_ptr->cipher_suite = CONFIG_CIPHER_OPEN;
	}
	return ret;
}

int wpa_config_set_wep(struct wpa_ssid *ssid)
{
	int ret = 0;
	char buf[20];
	
	if(g_sta_param_ptr->key.length == 5 ||
		g_sta_param_ptr->key.length == 13){
		if(wpa_config_set(ssid, "wep_key0", 
			str_format(buf, (char *)g_sta_param_ptr->key.array, g_sta_param_ptr->key.length), 0) < 0){
			ret++;
		}
	}else if(g_sta_param_ptr->key.length == 10 ||
				g_sta_param_ptr->key.length == 26){
		if(wpa_config_set(ssid, "wep_key0", (char *)g_sta_param_ptr->key.array, 0) < 0){
			ret++;
		}
	}else{
		ret++;
	}

	if(wpa_config_set(ssid, "key_mgmt", "NONE", 0) < 0){
		ret++;
	}

	if(wpa_config_set(ssid, "wep_tx_keyidx", "0", 0) < 0){
		ret++;
	}
	
	if(wpa_config_set(ssid, "auth_alg", "SHARED", 0) < 0){
		ret++;
	}

	if(wpa_config_set(ssid, "mem_only_psk", "0", 0) < 0){
		ret++;
	}

	if(!ret){
		g_sta_param_ptr->cipher_suite = CONFIG_CIPHER_WEP;
	}

	return ret;
}

int wpa_config_set_tkip(struct wpa_ssid *ssid)
{
	int ret = 0;
	char buf[20];
	
	if(wpa_config_set(ssid, "psk", 
		str_format(buf, (char *)g_sta_param_ptr->key.array, g_sta_param_ptr->key.length), 0) < 0){
		ret++;
	}
	if(wpa_config_set(ssid, "key_mgmt", "WPA-PSK", 0) < 0){
		ret++;
	}
	if(wpa_config_set(ssid, "proto", "WPA", 0) < 0){
		ret++;
	}
	if(wpa_config_set(ssid, "pairwise", "TKIP", 0) < 0){
		ret++;
	}
	if(wpa_config_set(ssid, "group", "TKIP", 0) < 0){
		ret++;
	}

	if(wpa_config_set(ssid, "mem_only_psk", "0", 0) < 0){
		ret++;
	}

	if(!ret){
		g_sta_param_ptr->cipher_suite = CONFIG_CIPHER_TKIP;
		wpa_config_update_psk(ssid);
	}

	return ret;
}

int wpa_config_set_ccmp(struct wpa_ssid *ssid)
{
	int ret = 0;
	char buf[20];

	if(wpa_config_set(ssid, "psk", 
		str_format(buf, (char *)g_sta_param_ptr->key.array, g_sta_param_ptr->key.length), 0) < 0){
		ret++;
	}
	if(wpa_config_set(ssid, "key_mgmt", "WPA-PSK", 0) < 0){
		ret++;
	}
	if(wpa_config_set(ssid, "proto", "RSN", 0) < 0){
		ret++;
	}
	if(wpa_config_set(ssid, "pairwise", "CCMP", 0) < 0){
		ret++;
	}
	if(wpa_config_set(ssid, "group", "CCMP", 0) < 0){
		ret++;
	}

	if(wpa_config_set(ssid, "mem_only_psk", "0", 0) < 0){
		ret++;
	}

	if(!ret){
		g_sta_param_ptr->cipher_suite = CONFIG_CIPHER_CCMP;
		wpa_config_update_psk(ssid);
	}

	return ret;
}

int wpa_config_set_wpa2mixed(struct wpa_ssid *ssid)
{
	int ret = 0;
	char buf[20];

	if(wpa_config_set(ssid, "psk", 
		str_format(buf, (char *)g_sta_param_ptr->key.array, g_sta_param_ptr->key.length), 0) < 0){
		ret++;
	}
	if(wpa_config_set(ssid, "key_mgmt", "WPA-PSK", 0) < 0){
		ret++;
	}
	if(wpa_config_set(ssid, "proto", "WPA RSN", 0) < 0){
		ret++;
	}
	if(wpa_config_set(ssid, "pairwise", "TKIP CCMP", 0) < 0){
		ret++;
	}
	if(wpa_config_set(ssid, "group", "TKIP", 0) < 0){
		ret++;
	}

	if(wpa_config_set(ssid, "mem_only_psk", "0", 0) < 0){
		ret++;
	}

	if(!ret){
		g_sta_param_ptr->cipher_suite = CONFIG_CIPHER_MIXED;
		wpa_config_update_psk(ssid);
	}

	return ret;
}


static struct wpa_ssid * wpa_config_read_network(int *line, int id)
{
	struct wpa_ssid *ssid;
	int errors = 0;
	char buf[20];

	ssid = os_zalloc(sizeof(*ssid));
	if (ssid == NULL)
		return NULL;
	
	dl_list_init(&ssid->psk_list);
	ssid->id = id;

	wpa_config_set_network_defaults(ssid);

	if(wpa_config_set(ssid, "ssid", 
		str_format(buf, (char *)g_sta_param_ptr->ssid.array, g_sta_param_ptr->ssid.length), *line) < 0)
	{
		errors++;
	}

	if(g_sta_param_ptr->fast_connect_set){
		sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x",
			g_sta_param_ptr->fast_connect.bssid[0], g_sta_param_ptr->fast_connect.bssid[1],
			g_sta_param_ptr->fast_connect.bssid[2], g_sta_param_ptr->fast_connect.bssid[3],
			g_sta_param_ptr->fast_connect.bssid[4], g_sta_param_ptr->fast_connect.bssid[5]);
		if(wpa_config_set(ssid, "bssid", buf, *line) < 0){
			errors++;
		}

		if(g_sta_param_ptr->cipher_suite == CONFIG_CIPHER_OPEN){
			errors += wpa_config_set_none(ssid);
		}else if(g_sta_param_ptr->cipher_suite == CONFIG_CIPHER_WEP){
			errors += wpa_config_set_wep(ssid);
		}else if(g_sta_param_ptr->cipher_suite == CONFIG_CIPHER_TKIP){
			errors += wpa_config_set_tkip(ssid);
		}else if(g_sta_param_ptr->cipher_suite == CONFIG_CIPHER_CCMP){
			errors += wpa_config_set_ccmp(ssid);
		}else if(g_sta_param_ptr->cipher_suite == CONFIG_CIPHER_MIXED){
			errors += wpa_config_set_wpa2mixed(ssid);
		}
	}else{
		if(wpa_config_set(ssid, "mem_only_psk", "1", *line) < 0){
			errors++;
		}
	}

	errors += wpa_config_validate_network(ssid, *line);

	if(errors){
		wpa_config_free_ssid(ssid);
		ssid = NULL;
	}

	return ssid;
}

#if 0
static struct wpa_ssid * wpa_config_read_network(int *line, int id)
{
	struct wpa_ssid *ssid;
	int errors = 0;
#if CFG_MODE_SWITCH
	char buf[20];
#endif
	
	ssid = os_zalloc(sizeof(*ssid));
	if (ssid == NULL)
		return NULL;
	
	dl_list_init(&ssid->psk_list);
	ssid->id = id;

	wpa_config_set_network_defaults(ssid);
#if CFG_WIFI_STATION_MODE
#if CFG_MODE_SWITCH
	if(wpa_config_set(ssid, "ssid", 
		str_format(buf, (char *)g_sta_param_ptr->ssid.array, g_sta_param_ptr->ssid.length), *line) < 0)
#else
	if(wpa_config_set(ssid, "ssid", "\""CFG_OOB_CONNECT_SSID"\"", *line) < 0)
#endif
	{
		errors++;
	}
#if 1
	if(g_sta_param_ptr->cipher_suite == CONFIG_CIPHER_OPEN){
		if(wpa_config_set(ssid, "key_mgmt", "NONE", *line) < 0){
			errors++;
		}
	}else if(g_sta_param_ptr->cipher_suite == CONFIG_CIPHER_WEP){
		if(wpa_config_set(ssid, "key_mgmt", "NONE", *line) < 0){
			errors++;
		}
		if(g_sta_param_ptr->key.length == 26){
			if(wpa_config_set(ssid, "wep_key0", (char *)g_sta_param_ptr->key.array, *line) < 0){
				errors++;
			}
		}else{
			if(wpa_config_set(ssid, "wep_key0", 
				str_format(buf, (char *)g_sta_param_ptr->key.array, g_sta_param_ptr->key.length), *line) < 0){
				errors++;
			}
		}
		if(wpa_config_set(ssid, "wep_tx_keyidx", "0", *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "auth_alg", "SHARED", *line) < 0){
			errors++;
		}
	}else if(g_sta_param_ptr->cipher_suite == CONFIG_CIPHER_CCMP){
		if(wpa_config_set(ssid, "key_mgmt", "WPA-PSK", *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "psk", 
			str_format(buf, (char *)g_sta_param_ptr->key.array, g_sta_param_ptr->key.length), *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "proto", "WPA RSN", *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "pairwise", "TKIP CCMP", *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "group", "TKIP CCMP", *line) < 0){
			errors++;
		}
	}else if(g_sta_param_ptr->cipher_suite == CONFIG_CIPHER_AUTO){
		if(wpa_config_set(ssid, "key_mgmt", "NONE WPA-PSK", *line) < 0){
			errors++;
		}

		if(g_sta_param_ptr->key.length == 13){
			if(wpa_config_set(ssid, "wep_key0", 
				str_format(buf, (char *)g_sta_param_ptr->key.array, g_sta_param_ptr->key.length), *line) < 0){
				errors++;
			}
		}else{
			if(wpa_config_set(ssid, "wep_key0", (char *)g_sta_param_ptr->key.array, *line) < 0){
				errors++;
			}
		}
		if(wpa_config_set(ssid, "wep_tx_keyidx", "0", *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "auth_alg", "SHARED", *line) < 0){
			errors++;
		}
		
		if(wpa_config_set(ssid, "psk", 
			str_format(buf, (char *)g_sta_param_ptr->key.array, g_sta_param_ptr->key.length), *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "proto", "WPA RSN", *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "pairwise", "TKIP CCMP", *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "group", "TKIP CCMP", *line) < 0){
			errors++;
		}
	}
#else
#if CFG_MODE_SWITCH
	if(g_sta_param_ptr->cipher_suite < CONFIG_CIPHER_TKIP){
		if(wpa_config_set(ssid, "key_mgmt", "NONE", *line) < 0){
			errors++;
		}
	}else{
		if(wpa_config_set(ssid, "psk", 
			str_format(buf, (char *)g_sta_param_ptr->key.array, g_sta_param_ptr->key.length), *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "key_mgmt", "WPA-PSK", *line) < 0){
			errors++;
		}
	}
	
	if(g_sta_param_ptr->cipher_suite == CONFIG_CIPHER_WEP){
		if(wpa_config_set(ssid, "wep_key0", (char *)g_sta_param_ptr->key.array, *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "wep_tx_keyidx", "0", *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "auth_alg", "SHARED", *line) < 0){
			errors++;
		}
	}else if(g_sta_param_ptr->cipher_suite == CONFIG_CIPHER_TKIP){
		if(wpa_config_set(ssid, "proto", "WPA", *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "pairwise", "TKIP", *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "group", "TKIP", *line) < 0){
			errors++;
		}
	}else if(g_sta_param_ptr->cipher_suite == CONFIG_CIPHER_CCMP){
		if(wpa_config_set(ssid, "proto", "WPA RSN", *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "pairwise", "TKIP CCMP", *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "group", "TKIP CCMP", *line) < 0){
			errors++;
		}
	}
	if(wpa_config_set(ssid, "priority", "0", *line) < 0){
		errors++;
	}
#else
#if (CFG_WIFI_WEP == 0 && CFG_WIFI_WPA == 0)
	if(wpa_config_set(ssid, "scan_ssid", "1", *line) < 0){
		errors++;
	}
	if(wpa_config_set(ssid, "key_mgmt", "NONE", *line) < 0){
	    errors++;
	}
	if(wpa_config_set(ssid, "priority", "0", *line) < 0){
	    errors++;
	}
#endif
#if CFG_WIFI_WEP
	if(wpa_config_set(ssid, "key_mgmt", "NONE", *line) < 0){
		errors++;
	}
	if(wpa_config_set(ssid, "wep_key0", CFG_WEP_KEY, *line) < 0){
		errors++;
	}
	if(wpa_config_set(ssid, "wep_tx_keyidx", "0", *line) < 0){
		errors++;
	}
	if(wpa_config_set(ssid, "auth_alg", "SHARED", *line) < 0){
		errors++;
	}
#endif
#if CFG_WIFI_WPA
		if(wpa_config_set(ssid, "psk", "\""CFG_WPA_KEY"\"", *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "key_mgmt", "WPA-PSK", *line) < 0){
			errors++;
		}
#if (CFG_WPA_TYPE == 0)
		if(wpa_config_set(ssid, "proto", "WPA", *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "pairwise", "TKIP", *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "group", "TKIP", *line) < 0){
			errors++;
		}
#else
		if(wpa_config_set(ssid, "proto", "RSN", *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "pairwise", "CCMP", *line) < 0){
			errors++;
		}
		if(wpa_config_set(ssid, "group", "CCMP", *line) < 0){
			errors++;
		}
#endif
		if(wpa_config_set(ssid, "priority", "99", *line) < 0){
			errors++;
		}
#endif
#endif
#endif
#endif
	errors += wpa_config_validate_network(ssid, *line);
	
	if(errors){
		wpa_config_free_ssid(ssid);
		ssid = NULL;
	}

	return ssid;
}
#endif

struct wpa_config * wpa_config_read(const char *name, struct wpa_config *cfgp)
{
	struct wpa_config *config;
	struct wpa_ssid *ssid, *tail, *head;
	int id = 0, line = 0, errors = 0;

	if (cfgp)
		config = cfgp;
	else
		config = wpa_config_alloc_empty(NULL, NULL);
	if (config == NULL)
		return NULL;
	tail = head = config->ssid;
	while (tail && tail->next)
		tail = tail->next;
	
	ssid = wpa_config_read_network(&line, id++);
	if(ssid == NULL){
		errors++;
		goto error;
	}
	if (head == NULL) {
		head = tail = ssid;
	} else {
		tail->next = ssid;
		tail = ssid;
	}
	if(wpa_config_add_prio_network(config, ssid)){
		errors++;
		goto error;
	}

	config->ssid = head;
	wpa_config_debug_dump_networks(config);

error:
	if(errors){
		wpa_config_free(config);
		config = NULL;
		head = NULL;
	}
	return config;
}


int wpa_config_write(const char *name, struct wpa_config *config)
{
	struct wpa_ssid *ssid;
	struct wpa_config_blob *blob;

	wpa_printf(MSG_DEBUG, "Writing configuration file '%s'", name);

	/* TODO: write global config parameters */


	for (ssid = config->ssid; ssid; ssid = ssid->next) {
		/* TODO: write networks */
	}

	for (blob = config->blobs; blob; blob = blob->next) {
		/* TODO: write blobs */
	}

	return 0;
}
