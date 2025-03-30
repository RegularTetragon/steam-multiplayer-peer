#include "multiplex_network.h"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/multiplayer_peer.hpp"
#include "godot_cpp/core/error_macros.hpp"

using namespace godot;

MultiplexNetwork::MultiplexNetwork(Ref<MultiplayerPeer> interface) {
    this->interface = interface;
}

Error MultiplexNetwork::send(Ref<MultiplexPacket> packet, int32_t peer_id, int32_t channel, MultiplayerPeer::TransferMode transfer_mode) {
    if (this->internal_peers.has(peer_id)) {
        Ref<MultiplexPeer> peer;
        peer = this->internal_peers.get(peer_id);
        peer->_put_multiplex_packet_direct(packet);
        return OK;
    } else if (this->external_peers.has(peer_id)) {
        this->interface->set_transfer_mode(transfer_mode);
        this->interface->set_target_peer(peer_id);
        this->interface->set_transfer_channel(channel);
        return this->interface->put_packet(packet->serialize());
    } else {
        ERR_FAIL_V_MSG(godot::ERR_CANT_CONNECT, "No known peer for peer_id");
    }
}

bool MultiplexNetwork::is_peer_connected(int32_t mux_peer_id) {
    return this->internal_peers.has(mux_peer_id) || this->external_peers.has(mux_peer_id);
}


Error MultiplexNetwork::disconnect_peer(int32_t mux_peer_id, bool force) {
    ERR_FAIL_COND_V_MSG(!this->internal_peers.has(mux_peer_id), ERR_CANT_CONNECT, "Cannot close peer that is not connected locally");
    
    Ref<MultiplexPeer> peer = this->internal_peers.get(mux_peer_id);
    peer->close();
    return OK;
}

void MultiplexNetwork::poll() {
    this->interface->poll();
    for (int i = 0; i < this->interface->get_available_packet_count(); i++) {
        PackedByteArray packet = this->interface->get_packet();
        Ref<MultiplexPacket> multiplex_packet = Ref<MultiplexPacket>(memnew(MultiplexPacket));
        Error error = multiplex_packet->deserialize(packet);
        ERR_CONTINUE_MSG(
            error != OK, "Deserializing packet failed." 
        );
        ERR_CONTINUE_MSG(
            !internal_peers.has(multiplex_packet->mux_peer_dest),
            "Multiplex destination peer id is not available locally"
        );
        ERR_CONTINUE_MSG(
            !external_peers.has(multiplex_packet->mux_peer_source),
            "Multiplex source peer id is not associated with any remote interface"
        );
        ERR_CONTINUE_MSG(
            external_peers.get(multiplex_packet->mux_peer_source) != this->interface->get_packet_peer(),
            "Multiplex source peer id is not associated with the provided interfaace peer id. Possible attempt at cheating."
        );
        internal_peers.get(multiplex_packet->mux_peer_dest)->_put_multiplex_packet_direct(multiplex_packet);
    }
}
