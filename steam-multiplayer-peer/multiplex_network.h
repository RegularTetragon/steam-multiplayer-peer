#ifndef MULTIPLEX_NETWORK_H
#define MULTIPLEX_NETWORK_H
#include "godot_cpp/classes/ref_counted.hpp"
#include "multiplex_packet.h"
#include "multiplex_peer.h"
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/classes/multiplayer_peer.hpp>

using namespace godot;

class MultiplexPeer;
class MultiplexNetwork : public RefCounted {
  private:
    HashMap<int32_t, Ref<MultiplexPeer>> internal_peers;
    HashMap<int32_t, int32_t> external_peers;
    Ref<MultiplayerPeer> interface; 
  public:
    bool _add_mux_peer(Ref<MultiplexPeer> peer);
    bool _remove_mux_peer(Ref<MultiplexPeer> peer);
    MultiplexNetwork(Ref<MultiplayerPeer> interface);
    ~MultiplexNetwork();
    Error send(Ref<MultiplexPacket> packet, int32_t peer_id, int32_t channel, MultiplayerPeer::TransferMode transfer_mode);
    bool is_peer_connected(int32_t mux_peer_id);
    Error disconnect_peer(int32_t mux_peer_id, bool force);
    void poll(); // responsible for taking packets off of interface, validating them, handling commands, and putting data packets into the correct MultiplexPeer queue
    Ref<MultiplayerPeer> _get_interface();
};
#endif
