#ifndef BLIZZARD_STRUCTS_H
#define BLIZZARD_STRUCTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <inttypes.h>

typedef struct BlizzardPacket{
  uint8_t _header[3];
  uint8_t _opcode;
  uint16_t _data_count;
  uint8_t _data[DMX_MAX_SLOTS];
}__attribute__((packed)) BlizzardPacket;

#ifdef __cplusplus
}
#endif

#endif
