#ifndef MULTIPLEX_PACKET_H
#define MULTIPLEX_PACKET_H

#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/ref_counted.hpp"
#include "godot_cpp/variant/packed_byte_array.hpp"
#include <cstdint>

enum MultiplexPacketSubtype{
  MUX_DATA,
  MUX_CMD
};

enum MultiplexPacketCommandSubtype{
  MUX_CMD_ADD_PEER_ACK,
  MUX_CMD_REMOVE_PEER_ACK,
  MUX_CMD_ADD_PEER_NACK,
  MUX_CMD_REMOVE_PEER_NACK,
  MUX_CMD_ADD_PEER,
  MUX_CMD_REMOVE_PEER,
};

struct MultiplexPacketCommand {
  MultiplexPacketCommandSubtype command;
  int32_t subject_multiplex_peer;
};

struct MultiplexPacketData {
  int32_t length;
  const uint8_t* data;
};

/*
 * serializes to
 * uint8_t subtype = 0x00
 * uint32_t muxpeer (net order)
 * uint32_t length  (net order)
 * uint8_t[length]
 * OR
 * uint8_t subtype = 0x01
 * uint8_t cmd = 0x00 NACK | 0x01 ACK | 0x02 ADD_PEER | 0x03 RM_PEER
 * uint32_t mux_peer [net order]
 * 
 */

struct MultiplexPacket : godot::RefCounted {
  MultiplexPacketSubtype subtype;
  int32_t mux_peer_source;
  int32_t mux_peer_dest;
  union {
    MultiplexPacketCommand command;
    MultiplexPacketData data;
  } contents;
  public:
    // wrap external packet into this mux packet structure
    void wrap(uint8_t *in, uint32_t len, uint32_t mux_peer);
    // remove multiplex data from a packet
    uint32_t unwrap(uint8_t **out, uint32_t len);
    // converts a multiplex packet into a newly allocated byte buffer in network order, returns length
    godot::PackedByteArray serialize();
    // converts a byte buffer into a multiplex packet, returns success or error
    godot::Error deserialize(godot::PackedByteArray rawData);
};
#endif
