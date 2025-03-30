# WIP

This fork is in heavy development. It is not in a functional state yet, please don't use it until it is ready.

# MultiplexPeer
The intention of this is to allow you to wrap arbitrary single multiplayer peers into many local multiplayer peers, utilizing said arbitrary multiplayer peer as a gateway to the larger network.

The motivation of this is that there are some multiplayer peer extensions which assume there is only, say, one MultiplayerPeer per Steam client.
ENetMultiplayerPeer makes no such assumptions and I have written a game which utilizes multiple ENetMultiplayerPeers in a single scene tree to allow for simplified game logic (a peer can only be a client or a server, never both) and easy split screen multiplayer (a server peer and multiple client peers may be present in a single scene tree).
Every existing SteamMultiplayerPeer as far as I can tell DOES make this assumption however. You can essentially only have one SteamMultiplayerPeer per Steam client. While this doesn't solve the case of having multiple windows open, it does solve both the split screen multiplayer and having entirely separate server/client logic.

This is a fork of ExpressoBit's SteamMultiplayerPeer. That said it will in a future version likely not contain any SteamMultiplayerPeer code. The intention is that this will be usable in conjunction with any other extension peer, not just SteamMultiplayerPeer.

This extension introduces two objects for the end user, a MultiplexNetwork, and MultiplexPeer. MultiplexNetwork is a local network that wraps another peer to act as a gateway to the larger network. The resulting network looks something like this:

```
MultiplexPeer (Client mpid 2) -\                                                                                                                   /- MultiplexPeer (Client mpid 4)
MultiplexPeer (Server mpid 1) -- MultiplexNetwork - MultiplayerPeer (Server mpid 1) - Network - MultiplayerPeer (Client mpid 2) - MultiplexNetwork -- MultiplexPeer (Client mpid 5)
MultiplexPeer (Client mpid 3) -/                                                                                                                   \- MultiplexPeer (Client mpid 6)
```

Each side of the network must wrap the same MultiplayerPeer in a multiplex network for this to work.

In terms of end user scripting, the exact interface isn't stable yet, but will end up being something akin to this

```gdscript
# starting a server
var gateway_peer = SteamMultiplayerPeer.new()
gateway_peer.start_server()

var mux_net = MultiplexNetwork.new(gateway_peer)

var mux_server = MultiplexPeer.new()
mux_server.start_server(mux_net)
instantiate_server(mux_server)

for i in range(1,split_screen_peers):
  var mux_peer = MultiplexPeer.new() 
  mux_peer.start_client(mux_net)
  instantiate_client(muxpeer)



# starting a server
var gateway_peer = SteamMultiplayerPeer.new()
gateway_peer.start_client(lobby_id)

var mux_net = MultiplexNetwork.new(gateway_peer)

for i in range(1,split_screen_peers):
  var mux_peer = MultiplexPeer.new() 
  mux_peer.start_client(mux_net)
  instantiate_client(mux_peer)
```

Where singleplayer is the case where split_screen_peers = 1.

There may be an easier way to do this in the future for SteamMultiplayerPeer in the future, however as of now it seems like this is the path of least resistance to get this particular usecase working.
