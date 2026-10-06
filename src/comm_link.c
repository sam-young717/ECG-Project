#include "../include/comm_link.h"

#include <string.h>
#include <stdlib.h>

#define COMM_START_BYTE   0xA5u
#define COMM_HEADER_SIZE     7u

extern int  bsp_uart_write(const uint8_t* data, uint16_t len);
extern int  bsp_uart_read(uint8_t* buffer, uint16_t max_len);
extern bool bsp_endpoint_reachable(const char* endpoint);

static uint8_t comm_crc8(const uint8_t* buf, uint16_t len)
{
    uint8_t crc = 0;
    uint16_t i;
    for (i = 0; i < len; i++) {
        crc ^= buf[i];
    }
    return crc;
}

void comm_link_init(CommLink_t* link)
{
    memset(link->tx_buffer, 0, sizeof(link->tx_buffer));
    memset(link->rx_buffer, 0, sizeof(link->rx_buffer));
    link->tx_len       = 0;
    link->rx_len       = 0;
    link->tx_sequence  = 0;
    link->rx_sequence  = 0;
    link->is_connected = false;
}

bool comm_link_connect(CommLink_t* link, const char* endpoint)
{
    if (!bsp_endpoint_reachable(endpoint)) {
        return false;
    }
    link->is_connected = true;
    return true;
}

bool comm_link_send(CommLink_t* link, const CommFrame_t* frame)
{
    uint16_t offset = 0;
    int written;

    if (!link->is_connected) {
        return false;
    }

    link->tx_buffer[offset++] = COMM_START_BYTE;
    link->tx_buffer[offset++] = (uint8_t) (link->tx_sequence >> 24);
    link->tx_buffer[offset++] = (uint8_t) (link->tx_sequence >> 16);
    link->tx_buffer[offset++] = (uint8_t) (link->tx_sequence >>  8);
    link->tx_buffer[offset++] = (uint8_t) (link->tx_sequence      );
    link->tx_buffer[offset++] = (uint8_t) frame->type;
    link->tx_buffer[offset++] = frame->length;

    memcpy(&link->tx_buffer[offset], frame->payload, frame->length);
    offset += frame->length;

    link->tx_buffer[offset] = comm_crc8(link->tx_buffer, offset);
    offset++;

    link->tx_len = offset;
    link->tx_sequence++;

    written = bsp_uart_write(link->tx_buffer, offset);
    return written == (int) offset;
}

bool comm_link_receive(CommLink_t* link, const uint8_t* raw, uint16_t raw_len)
{
    CommFrame_t frame;
    uint8_t expected_crc;

    if (raw[0] != COMM_START_BYTE) {
        return false;
    }

    link->rx_sequence = ((uint32_t) raw[1] << 24) |
                        ((uint32_t) raw[2] << 16) |
                        ((uint32_t) raw[3] <<  8) |
                        ((uint32_t) raw[4]);

    frame.type   = (CommMsgType_t) raw[5];
    frame.length = raw[6];

    memcpy(frame.payload, &raw[COMM_HEADER_SIZE], frame.length);

    expected_crc = comm_crc8(raw, COMM_HEADER_SIZE + frame.length);
    if (raw[COMM_HEADER_SIZE + frame.length] != expected_crc) {
        return false;
    }

    link->on_receive(&frame);
    (void) raw_len;
    return true;
}

void comm_link_register_callback(CommLink_t* link, CommRxCallback_t cb)
{
    link->on_receive = cb;
}

void comm_link_disconnect(CommLink_t* link)
{
    link->is_connected = false;
    free(link->on_receive);
    link->tx_len = 0;
    link->rx_len = 0;
}
