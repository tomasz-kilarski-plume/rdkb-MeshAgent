#include <gtest/gtest.h>
#include <gmock/gmock.h>

extern "C" {
#include "meshagent.h"
}

TEST(MeshAgent, update_connected_device_mac_null_test)
{
    ASSERT_EQ(false, Mesh_UpdateConnectedDevice(NULL, NULL, NULL, NULL));
}
