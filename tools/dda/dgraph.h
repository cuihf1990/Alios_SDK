/*
 * Copyright (C) 2016 YunOS Project. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef DDA_DGRAPH_H
#define DDA_DGRAPH_H

int dgraph_sync_rssis(const char *rssi_metrics, unsigned int device_num);

int dgraph_create_udp_socket();

int dgraph_sync_device ( uint64_t agent_id,
                         int alive,
                         uint64_t mac_addr,
                         int channel,
                         const char *ip_addr,
                         unsigned int port,
                         unsigned int netid,
                         unsigned int sid,
                         unsigned int index );

int dgraph_append_behavior_index(uint64_t index_from,
                                 uint64_t index_to,
                                 uint64_t netid_from,
                                 uint64_t netid_to,
                                 uint16_t type,
                                 uint16_t cmd,
                                 const char *msg, unsigned int len);
int dgraph_clear_device();

#ifdef CONFIG_DDA_DGRAPH_ENABLE

#define __DDA_DGRAPH_INIT__ \
        dgraph_create_udp_socket();

#define __DDA_DGRAPH_SYNC_RSSI__(rssi, num) dgraph_sync_rssis(rssi, num)

#define __DDA_DGRAPH_SYNC_DEVICE__(agent_id, alive, mac_addr, channel, ip_addr, port, netid, sid, index) \
                                    dgraph_sync_device(agent_id, \
                                                        alive, \
                                                        mac_addr, \
                                                        channel, \
                                                        ip_addr, \
                                                        port, \
                                                        netid, sid, index)

#define __DDA_DGRAPH_CLEAR_DEVICE__() \
                                    dgraph_clear_device()

#define __DDA_DGRAPH_SYNC_MSG__(from, to, netid_from, netid_to, type, cmd, msg, len) \
                            dgraph_append_behavior_index(from, to,\
                                                        netid_from, netid_to,\
                                                        type, cmd, msg, len)

#else

#define __DDA_DGRAPH_INIT__
#define __DDA_DGRAPH_SYNC_RSSI__(rssi, num)
#define __DDA_DGRAPH_SYNC_DEVICE__(agent_id, alive, mac_addr, channel, ip_addr, port, netid, sid, index)
#define __DDA_DGRAPH_CLEAR_DEVICE__()
#define __DDA_DGRAPH_SYNC_MSG__(from, to, netid_from, netid_to, type, cmd, msg, len)
#endif

#endif /* DDA_DGRAPH_H */
