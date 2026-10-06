/*
 * comm_link.h
 *
 * Serial / network transport used to relay vitals to a central station.
 * Frames a simple TLV-style packet (type + length + payload) and provides
 * a receive-callback registration point for remote commands.
 */
#ifndef COMM_LINK_H
#define COMM_LINK_H

#include <stdint.h>
#include <stdbool.h>

#define COMM_TX_BUFFER_SIZE   256
#define COMM_RX_BUFFER_SIZE   256
#define COMM_MAX_PAYLOAD      200

typedef enum {
    COMM_MSG_VITALS      = 0x10,
    COMM_MSG_ALARM       = 0x20,
    COMM_MSG_ACK         = 0x30,
    COMM_MSG_CMD_QUERY   = 0x40,
    COMM_MSG_CMD_SILENCE = 0x41
} CommMsgType_t;

typedef struct {
    CommMsgType_t type;
    uint8_t       length;
    uint8_t       payload[COMM_MAX_PAYLOAD];
} CommFrame_t;

typedef void (*CommRxCallback_t)(const CommFrame_t* frame);

typedef struct {
    uint8_t          tx_buffer[COMM_TX_BUFFER_SIZE];
    uint8_t          rx_buffer[COMM_RX_BUFFER_SIZE];
    uint16_t         tx_len;
    uint16_t         rx_len;
    uint32_t         tx_sequence;
    uint32_t         rx_sequence;
    CommRxCallback_t on_receive;
    bool             is_connected;
} CommLink_t;

void comm_link_init(CommLink_t* link);
bool comm_link_connect(CommLink_t* link, const char* endpoint);
bool comm_link_send(CommLink_t* link, const CommFrame_t* frame);
bool comm_link_receive(CommLink_t* link, const uint8_t* raw, uint16_t raw_len);
void comm_link_register_callback(CommLink_t* link, CommRxCallback_t cb);
void comm_link_disconnect(CommLink_t* link);

#endif /* COMM_LINK_H */
