#include <gtest/gtest.h>

extern "C" {
#include "MeshAgentSsp/ethpod_error_det.h"
}

TEST(EthPodErrDet, test1234)
{
    ASSERT_EQ(false, meshHandleEvent("d0:6e:de:5e:65:dc", DHCP_ACK_PRIV_EVENT));
    ASSERT_EQ(true, meshAddPod("d0:6e:de:5e:65:dc"));
    ASSERT_EQ(true, meshAddPod("d0:6e:de:5e:65:dc"));
    ASSERT_EQ(true, meshAddPod("d0:6e:de:5e:65:dd"));

    ASSERT_EQ(true, meshHandleEvent("d0:6e:de:5e:65:dc", DHCP_ACK_PRIV_EVENT)); //handle event okay
    ASSERT_EQ(false, meshHandleEvent("ab:6e:de:5e:65:dc", DHCP_ACK_PRIV_EVENT)); //No pod with this mac
}
