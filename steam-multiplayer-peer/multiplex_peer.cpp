#include "multiplex_peer.h"
#include "godot_cpp/classes/global_constants.hpp"
#include "godot_cpp/classes/multiplayer_peer.hpp"
#include "godot_cpp/core/error_macros.hpp"
#include "multiplex_packet.h"
#include "multiplex_network.h"
#include "steam_packet_peer.h"
#include <godot_cpp/variant/utility_functions.hpp>

MultiplexPeer::MultiplexPeer() {
    this->active_mode = MODE_NONE;
    this->target_peer = 0;
}
MultiplexPeer::~MultiplexPeer() {
    if(active_mode != MODE_NONE) {
        this->close();
    }
}
Error MultiplexPeer::create_server(Ref<MultiplexNetwork> network) {
    this->active_mode = MultiplexPeer::Mode::MODE_SERVER;
    this->network = network;
    network->_add_mux_peer(this);
    return OK;
}
Error MultiplexPeer::create_client(Ref<MultiplexNetwork> network) {
    this->active_mode = MultiplexPeer::Mode::MODE_CLIENT;
    this->network = network;
    network->_add_mux_peer(this);
    Ref<MultiplexPacket> create_client_cmd_packet = Ref<MultiplexPacket>(memnew(MultiplexPacket()));
    create_client_cmd_packet->subtype = MultiplexPacketSubtype::MUX_CMD;
    create_client_cmd_packet->mux_peer_source = _get_unique_id();
    create_client_cmd_packet->mux_peer_dest = 1;
    create_client_cmd_packet->contents.command.command = MultiplexPacketCommandSubtype::MUX_CMD_ADD_PEER;
    create_client_cmd_packet->contents.command.subject_multiplex_peer = 0;
    network->send(create_client_cmd_packet, 1, 0, TRANSFER_MODE_RELIABLE);

    return OK;
}
Error MultiplexPeer::_get_packet(const uint8_t **r_buffer, int32_t *r_buffer_size) {
    ERR_FAIL_COND_V_MSG(incoming_packets.size() == 0, ERR_UNAVAILABLE, "No incoming packets available.");

    current_packet = incoming_packets.front()->get();
    incoming_packets.pop_front();

    *r_buffer = current_packet->contents.data.data;
    *r_buffer_size = current_packet->contents.data.length;
    
    return OK;
}
Error MultiplexPeer::_put_packet(const uint8_t *p_buffer, int32_t p_buffer_size) {
    ERR_FAIL_COND_V_MSG(active_mode == MODE_NONE, ERR_UNCONFIGURED, "Peer is not in a MultiplexNetwork");
    ERR_FAIL_COND_V_MSG(
        this->target_peer == 0
        ||
        this->network->is_peer_connected(this->target_peer),
        ERR_UNAVAILABLE,
        "No known route to peer"
    );
    Ref<MultiplexPacket> packet = Ref<MultiplexPacket>(memnew(MultiplexPacket()));
    packet->subtype = MUX_DATA;
    packet->mux_peer_source = this->_get_unique_id();
    packet->mux_peer_dest = this->target_peer;
    packet->contents.data.length = p_buffer_size;
    packet->contents.data.data = p_buffer;
    return network->send(packet, target_peer, current_channel, current_transfer_mode);
}
void MultiplexPeer::_poll() {
    this->network->poll();
}
int32_t MultiplexPeer::_get_available_packet_count() const {
    return this->incoming_packets.size();
}

int32_t MultiplexPeer::_get_max_packet_size() const {
    return
        this->network.is_null() ? 0
      : this->network->_get_interface().is_null() ? 0
      : MAX_STEAM_PACKET_SIZE;
}

int32_t MultiplexPeer::_get_packet_channel() const {
    return this->current_channel;
}

void MultiplexPeer::_set_transfer_channel(int32_t value) {
    this->current_channel = value;
}

MultiplayerPeer::TransferMode MultiplexPeer::_get_transfer_mode() const {
    return this->current_transfer_mode;
}

void MultiplexPeer::_set_transfer_mode(MultiplayerPeer::TransferMode value) {
    this->current_transfer_mode = value;
}

bool MultiplexPeer::_is_server() const {
    return this->active_mode == MODE_SERVER;
}


int32_t MultiplexPeer::_get_packet_peer() const {
    ERR_FAIL_COND_V_MSG(this->active_mode == MODE_NONE, 1, "Multiplex peer not connected.");
    ERR_FAIL_COND_V_MSG(this->current_packet.is_null(), 1, "No packets available");
    return this->current_packet->mux_peer_source;
}

void MultiplexPeer::_close() {
    if (this->active_mode == MODE_NONE) {
        return;
    }
    Ref<MultiplexPacket> packet;
    packet->subtype = MUX_CMD;
    packet->mux_peer_source = this->_get_unique_id();
    packet->mux_peer_dest = 1;
    packet->contents.command.subject_multiplex_peer = this->_get_unique_id();
    packet->contents.command.command = MUX_CMD_REMOVE_PEER;
    incoming_packets.clear();
    this->network->send(packet, 1, 0, TRANSFER_MODE_RELIABLE);
    this->active_mode = MODE_NONE;
    this->network->_remove_mux_peer(this);
}

void MultiplexPeer::_disconnect_peer(int32_t p_peer, bool p_force) {
    this->network->disconnect_peer(p_peer, p_force);
}

MultiplayerPeer::ConnectionStatus MultiplexPeer::_get_connection_status() const {
    return connection_status;
}

bool MultiplexPeer::_is_server_relay_supported() const {
    ERR_FAIL_COND_V_MSG(this->network.is_null(), 0, "MultiplexPeer has no associated network.");
    ERR_FAIL_COND_V_MSG(this->network->_get_interface().is_null(), 0, "MultiplexNetwork has no interface");
    return this->network->_get_interface()->is_server_relay_supported();
}

Error MultiplexPeer::_put_multiplex_packet_direct(Ref<MultiplexPacket> packet) {
    this->incoming_packets.push_back(packet);
    return OK;
}
