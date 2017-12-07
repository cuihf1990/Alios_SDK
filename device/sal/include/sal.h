#ifndef _SAL_H_
#define _SAL_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    /* WiFi */
    TCP_SERVER,
    TCP_CLIENT,
    SSL_CLIENT,
    UDP_BROADCAST,
    UDP_UNICAST,
    /*WiFi end */
    /* Add others hereafter */
} CONN_TYPE;

/* Fill necessary fileds according to the socket type. */
typedef struct {
    int fd; /* fd that are used in socket level */
    CONN_TYPE type;
    char *addr; /* remote ip or domain */
    int32_t r_port; /* remote port (set to -1 if not used) */
    int32_t l_port; /* local port (set to -1 if not used) */
    uint32_t tcp_keep_alive; /* tcp keep alive value (set to 0 if not used) */
} sal_conn_t;

/* Socket data state indicator. */
typedef enum netconn_evt {
    NETCONN_EVT_RCVPLUS,
    NETCONN_EVT_RCVMINUS,
    NETCONN_EVT_SENDPLUS,
    NETCONN_EVT_SENDMINUS,
    NETCONN_EVT_ERROR
} netconn_evt_t;

typedef void (*netconn_evt_cb_t)(int s, enum netconn_evt evt);

typedef struct sal_op_s {
    char *version; /* Reserved for furture use. */

    /**
     * Module low level init so that it's ready to setup socket connection.
     *
     * @return  0 - success, -1 - failure
     */
    int (*init)(void);

    /**
     * Start a socket connection via module.
     *
     * @param[in]  c - connect parameters which are used to setup
     *                 the socket connection.
     *
     * @return  0 - success, -1 - failure
     */
    int (*start)(sal_conn_t *c);

    /**
     * Send data via module.
     * This function does not return until all data sent.
     *
     * @param[in]  fd - the file descripter to operate on.
     * @param[in]  data - pointer to data to send.
     * @param[in]  len - length of the data.
     * @param[in]  remote_ip - remote port number (optional).
     * @param[in]  remote_port - remote port number (optional).
     *
     * @return  0 - success, -1 - failure
     */
    int (*send)(int fd, uint8_t *data, uint32_t len,
                char remote_ip[16], int32_t remote_port);

    /**
     * Receive data from module.
     * This function returns either when expected length data read,
     * or no more data left. This OUT "len" reflects the real length read.
     * The caller is epected to check the returned len.
     *
     * @param[in]   fd - the file descripter to operate on.
     * @param[out]  data - pointer to data to send.
     * @param[out]  len - expected length of the data when IN,
     *                    and real read len when OUT.
     * @param[out]  remote_ip - remote ip address. Caller manages the
                                memory (optional).
     * @param[out]  remote_port - remote port number (optional).
     *
     * @return  0 - success, -1 - failure
     */
    int (*recv)(int fd, uint8_t *buf, uint32_t *len,
                char remote_ip[16], int32_t remote_port);

    /**
     * Get IP information of the corresponding domain.
     * Currently only one IP string is returned (even when the domain
     * coresponses to mutliple IPs). Note: only IPv4 is supported.
     *
     * @param[in]   domain - the domain string.
     * @param[out]  ip - the place to hold the dot-formatted ip string.
     *
     * @return  0 - success, -1 - failure
     */
    int (*domain_to_ip)(char *domain, char ip[16]);

    /**
     * Close the socket connection.
     *
     * @param[in]  fd - the file descripter to operate on.
     * @param[in]  remote_port - remote port number (optional).
     *
     * @return  0 - success, -1 - failure
     */
    int (*close)(int fd, int32_t remote_port);

    /**
     * Destroy SAL or exit low level state if necessary.
     *
     * @return  0 - success, -1 - failure
     */
    int (*deinit)(void);

    /**
     * Register network connection event callback function.
     * This callback should be called by following below rules:
     *
     *   1. When there is socket data available for a specific fd, call as:
     *      g_netconn_evt_cb(fd, NETCONN_EVT_RCVPLUS);
     *   2. When there is no more socket data for a specific fd, call as:
     *      g_netconn_evt_cb(fd, NETCONN_EVT_RCVMINUS);
     *   3. When a send action is finished for a specific fd, call as:
     *      g_netconn_evt_cb(fd, NETCONN_EVT_SENDPLUS);
     *   4. When there is an fatal error occured, call as:
     *      g_netconn_evt_cb(fd, NETCONN_EVT_ERROR);
     *   5. Whenever a socket is going to be closed, call as:
     *      g_netconn_evt_cb(fd, NETCONN_EVT_RCVPLUS);
     *      g_netconn_evt_cb(fd, NETCONN_EVT_SENDPLUS);
     *      g_netconn_evt_cb(fd, NETCONN_EVT_ERROR);
     */
    int (*register_netconn_evt_cb)(netconn_evt_cb_t cb);
} sal_op_t;


/**
 * Register a module instance to the SAL
 *
 * @param[in] module the module instance
**/
int sal_module_register(sal_op_t *module);

/**
 * Module low level init so that it's ready to setup socket connection.
 *
 * @return  0 - success, -1 - failure
 */
int sal_module_init(void);

/**
 * Start a socket connection via module.
 *
 * @param[in]  conn - connect parameters which are used to setup
 *                 the socket connection.
 *
 * @return  0 - success, -1 - failure
 */
int sal_module_start(sal_conn_t *conn);

/**
 * Send data via module.
 * This function does not return until all data sent.
 *
 * @param[in]  fd - the file descripter to operate on.
 * @param[in]  data - pointer to data to send.
 * @param[in]  len - length of the data.
 * @param[in]  remote_ip - remote port number (optional).
 * @param[in]  remote_port - remote port number (optional).
 *
 * @return  0 - success, -1 - failure
 */
int sal_module_send(int fd, uint8_t *data, uint32_t len,
                        char remote_ip[16], int32_t remote_port);

/**
 * Receive data from module.
 * This function returns either when expected length data read,
 * or no more data left. This OUT "len" reflects the real length read.
 * The caller is epected to check the returned len.
 *
 * @param[in]   fd - the file descripter to operate on.
 * @param[out]  data - pointer to data to send.
 * @param[out]  len - expected length of the data when IN,
 *                    and real read len when OUT.
 * @param[out]  remote_ip - remote ip address. Caller manages the
                            memory (optional).
 * @param[out]  remote_port - remote port number (optional).
 *
 * @return  0 - success, -1 - failure
 */
int sal_module_recv(int fd, uint8_t *buf, uint32_t *len,
                        char remote_ip[16], int32_t remote_port);

/**
 * Get IP information of the corresponding domain.
 * Currently only one IP string is returned (even when the domain
 * coresponses to mutliple IPs). Note: only IPv4 is supported.
 *
 * @param[in]   domain - the domain string.
 * @param[out]  ip - the place to hold the dot-formatted ip string.
 *
 * @return  0 - success, -1 - failure
 */
int sal_module_domain_to_ip(char *domain, char ip[16]);

/**
 * Close the socket connection.
 *
 * @param[in]  fd - the file descripter to operate on.
 * @param[in]  remote_port - remote port number (optional).
 *
 * @return  0 - success, -1 - failure
 */
int sal_module_close(int fd, int32_t remote_port);

/**
 * Destroy SAL or exit low level state if necessary.
 *
 * @return  0 - success, -1 - failure
 */
int sal_module_deinit(void);

/**
 * Register network connection event callback function.
 * This callback should be called by following below rules:
 *
 *   1. When there is socket data available for a specific fd, call as:
 *      g_netconn_evt_cb(fd, NETCONN_EVT_RCVPLUS);
 *   2. When there is no more socket data for a specific fd, call as:
 *      g_netconn_evt_cb(fd, NETCONN_EVT_RCVMINUS);
 *   3. When a send action is finished for a specific fd, call as:
 *      g_netconn_evt_cb(fd, NETCONN_EVT_SENDPLUS);
 *   4. When there is an fatal error occured, call as:
 *      g_netconn_evt_cb(fd, NETCONN_EVT_ERROR);
 *   5. Whenever a socket is going to be closed, call as:
 *      g_netconn_evt_cb(fd, NETCONN_EVT_RCVPLUS);
 *      g_netconn_evt_cb(fd, NETCONN_EVT_SENDPLUS);
 *      g_netconn_evt_cb(fd, NETCONN_EVT_ERROR);
 */
int sal_module_register_netconn_evt_cb(netconn_evt_cb_t cb);


#ifdef __cplusplus
}
#endif

#endif
