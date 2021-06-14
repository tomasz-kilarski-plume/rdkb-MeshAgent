#ifndef ETHPOD_ERROR_DET_H
#define ETHPOD_ERROR_DET_H

#include <stdbool.h>

typedef enum _eth_podevent {
    DHCP_ACK_PRIV_EVENT, //10.0.0.x ACK
    DHCP_ACK_VLAN_EVENT,
    DHCP_ACK_BHAUL_EVENT, //192.168.245.a ACK
    POD_DC_EVENT
} EthPodEvent;

bool meshAddPod(const char * pod_mac);
bool meshRemovePods();
bool meshHandleEvent(const char * pod_mac, EthPodEvent event);
void meshHandleTimeout();

#endif
