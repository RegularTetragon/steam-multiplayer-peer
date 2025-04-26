#ifndef MULTIPLEX_NETWORK_H
#define MULTIPLEX_NETWORK_H
#include "godot_cpp/classes/ref_counted.hpp"
#include "multiplex_packet.h"
#include <godot_cpp/classes/multiplayer_peer.hpp>
#include <godot_cpp/templates/hash_map.hpp>

using namespace godot;

class MultiplexPeer;
class MultiplexNetwork : public RefCounted {
	GDCLASS(MultiplexNetwork, RefCounted)
private:
	HashMap<int32_t, Ref<MultiplexPeer>> internal_peers;
	HashMap<int32_t, int32_t> external_peers;
	Ref<MultiplayerPeer> interface;
	uint32_t max_subpeers; // 0 = inf
	Error handle_command_dom(int32_t sender_pid, Ref<MultiplexPacket> packet);
	Error handle_command_sub(int32_t sender_pid, Ref<MultiplexPacket> packet);
  Error send_command(MultiplexPacketCommandSubtype subtype, int32_t subject_multiplex_peer, int32_t to_interface_pid);
protected:
  static void _bind_methods();
public:
  Ref<MultiplexPeer> create_server(uint32_t max_remote_subpeers);
  Ref<MultiplexPeer> create_client();

	Error _register_mux_peer(Ref<MultiplexPeer> peer);
	void _remove_mux_peer(Ref<MultiplexPeer> peer);
	~MultiplexNetwork();
  void set_interface(Ref<MultiplayerPeer> interface);
	Error send(Ref<MultiplexPacket> packet, int32_t peer_id, int32_t channel, MultiplayerPeer::TransferMode transfer_mode);
	bool is_peer_connected(int32_t mux_peer_id);
	Error disconnect_peer(int32_t mux_peer_id, bool force);
	void poll(); // responsible for taking packets off of interface, validating them, handling commands, and putting data packets into the correct MultiplexPeer queue, may be called multiple times in one frame
	Ref<MultiplayerPeer> _get_interface();
};
#endif
