//
//  ws.h
//  RadarHub
//
//  Description of the frame header using the official 
//  websocket document RFC6455, which can be found at
//  https://datatracker.ietf.org/doc/html/rfc6455
//
//  Created by Boonleng Cheong on 8/3/2021.
//  Copyright (c) 2021 Boonleng Cheong. All rights reserved.
//

#ifndef __rfc_6455__
#define __rfc_6455__

#include <stdint.h>
#include <stdbool.h>

typedef uint8_t RFC6455_OPCODE;
enum {
    RFC6455_OPCODE_CONTINUATION,
    RFC6455_OPCODE_TEXT,
    RFC6455_OPCODE_BINARY,
    RFC6455_OPCODE_RESERVE_3,
    RFC6455_OPCODE_RESERVE_4,
    RFC6455_OPCODE_RESERVE_5,
    RFC6455_OPCODE_RESERVE_6,
    RFC6455_OPCODE_RESERVE_7,
    RFC6455_OPCODE_CLOSE,
    RFC6455_OPCODE_PING,
    RFC6455_OPCODE_PONG,
    RFC6455_OPCODE_RESERVE_B,
    RFC6455_OPCODE_RESERVE_C,
    RFC6455_OPCODE_RESERVE_D,
    RFC6455_OPCODE_RESERVE_E,
    RFC6455_OPCODE_RESERVE_F
};

#define OPCODE_STRING(x) \
(x == RFC6455_OPCODE_PING ? "PING" : \
(x == RFC6455_OPCODE_PONG ? "PONG" : "Payload"))

typedef union {
    uint32_t u32;
    uint8_t code[4];
} ws_mask_key;

#pragma pack(push, 1)

//
//
//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-------+-+-------------+-------------------------------+
//  |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
//  |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
//  |N|V|V|V|       |S|             |   (if payload len==126/127)   |
//  | |1|2|3|       |K|             |                               |
//  +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
//  |     Extended payload length continued, if payload len == 127  |
//  + - - - - - - - - - - - - - - - +-------------------------------+
//  |                               |Masking-key, if MASK set to 1  |
//  +-------------------------------+-------------------------------+
//  | Masking-key (continued)       |          Payload Data         |
//  +-------------------------------- - - - - - - - - - - - - - - - +
//  :                     Payload Data continued ...                :
//  + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
//  |                     Payload Data continued ...                |
//  +---------------------------------------------------------------+
//
//
//  Note LSB is on the left. Need to mirror it at each byte.
//

typedef union {
    struct {
        uint8_t opcode:4;                     // Word 0 bits 4-7
        uint8_t rsv:3;                        // Word 0 bits 1-3
        bool fin:1;                           // Word 0 bit 0
        uint8_t len:7;                        // Word 0 bits 9-15
        bool mask:1;                          // Word 0 bit 8
    };
    uint8_t bytes[2];
} ws_frame_header;

#pragma pack(pop)

#endif // defined(__rfc_6455__)
