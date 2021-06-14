#include <gtest/gtest.h>

extern "C" {
#include "ethpod_error_det.h"
}

TEST(EthPodErrDet, test123)
{
    ASSERT_EQ(1+1, 2);
}

TEST(EthPodErrDet, test1234)
{
    ASSERT_EQ(-1, meshHandleEvent("d0:6e:de:5e:65:dc", DHCP_ACK_PRIV_EVENT));
    ASSERT_EQ(0, meshAddPod("d0:6e:de:5e:65:dc"));
    ASSERT_EQ(0, meshAddPod("d0:6e:de:5e:65:dc"));
    ASSERT_EQ(0, meshAddPod("d0:6e:de:5e:65:dd"));

    ASSERT_EQ(0, meshHandleEvent("d0:6e:de:5e:65:dc", DHCP_ACK_PRIV_EVENT)); //handle event okay
    ASSERT_EQ(-1, meshHandleEvent("ab:6e:de:5e:65:dc", DHCP_ACK_PRIV_EVENT)); //No pod with this mac
}
