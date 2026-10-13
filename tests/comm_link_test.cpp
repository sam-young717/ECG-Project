#include <gtest/gtest.h>

#include <cstring>

extern "C" {
#include "comm_link.h"
}

namespace {
bool endpoint_reachable;
uint8_t transmitted[COMM_TX_BUFFER_SIZE];
uint16_t transmitted_length;
CommFrame_t received;
int callback_count;

void receive_callback(const CommFrame_t* frame)
{
    received = *frame;
    ++callback_count;
}
}

extern "C" int bsp_uart_write(const uint8_t* data, uint16_t length)
{
    std::memcpy(transmitted, data, length);
    transmitted_length = length;
    return length;
}

extern "C" int bsp_uart_read(uint8_t*, uint16_t) { return 0; }
extern "C" bool bsp_endpoint_reachable(const char*) { return endpoint_reachable; }

TEST(CommLink, ConnectsOnlyWhenTheEndpointIsReachable)
{
    CommLink_t link;
    comm_link_init(&link);
    endpoint_reachable = false;
    EXPECT_FALSE(comm_link_connect(&link, "central"));
    endpoint_reachable = true;
    EXPECT_TRUE(comm_link_connect(&link, "central"));
    EXPECT_TRUE(link.is_connected);
}

TEST(CommLink, SerializesFramesAndAdvancesTheSequence)
{
    CommLink_t link;
    CommFrame_t frame = {COMM_MSG_VITALS, 2, {0x11, 0x22}};
    comm_link_init(&link);
    endpoint_reachable = true;
    ASSERT_TRUE(comm_link_connect(&link, "central"));

    ASSERT_TRUE(comm_link_send(&link, &frame));
    EXPECT_EQ(transmitted_length, 10);
    EXPECT_EQ(transmitted[0], 0xA5);
    EXPECT_EQ(transmitted[5], COMM_MSG_VITALS);
    EXPECT_EQ(transmitted[6], 2);
    EXPECT_EQ(transmitted[7], 0x11);
    EXPECT_EQ(transmitted[8], 0x22);
    EXPECT_EQ(link.tx_sequence, 1u);
}

TEST(CommLink, DeliversValidFramesAndRejectsBadCrc)
{
    CommLink_t link;
    uint8_t frame[] = {0xA5, 0, 0, 0, 9, COMM_MSG_ACK, 1, 0x44, 0};
    for (size_t i = 0; i < sizeof(frame) - 1; ++i) {
        frame[sizeof(frame) - 1] ^= frame[i];
    }
    callback_count = 0;
    comm_link_init(&link);
    comm_link_register_callback(&link, receive_callback);

    ASSERT_TRUE(comm_link_receive(&link, frame, sizeof(frame)));
    EXPECT_EQ(link.rx_sequence, 9u);
    EXPECT_EQ(callback_count, 1);
    EXPECT_EQ(received.payload[0], 0x44);
    frame[8] ^= 0x01;
    EXPECT_FALSE(comm_link_receive(&link, frame, sizeof(frame)));

    comm_link_disconnect(&link);
    EXPECT_EQ(link.on_receive, nullptr);
}
