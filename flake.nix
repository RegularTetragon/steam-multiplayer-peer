{
  inputs = {
    nixpkgs = {
      type = "github";
      owner = "NixOS";
      repo = "nixpkgs";
      ref = "nixos-unstable";
    };
    godot-cpp = {
      flake = false;
      type = "github";
      owner = "godotengine";
      repo = "godot-cpp";
      ref = "f3a1a2fd458dfaf4de08c906f22a2fe9e924b16f";
    };
  };
  outputs = {self, nixpkgs, godot-cpp, ...}:
  let system = "x86_64-linux";
      pkgs = nixpkgs.legacyPackages.${system};
      stdenv = pkgs.stdenv;
  in {
    packages.${system} = {
      default = self.packages.${system}.godot-multiplayer-peer;
      godot-cpp = let
        mkSconsFlagsFromAttrSet = pkgs.lib.mapAttrsToList (
          k: v: if builtins.isString v then "${k}=${v}" else "${k}=${builtins.toJSON v}"
        );
      in with stdenv; mkDerivation rec {
        pname = "godot-cpp";
        version = "1.0.0";
        nativeBuildInputs = with pkgs; [scons pkgconf libgcc xorg.libXcursor xorg.libXinerama xorg.libXi xorg.libXrandr wayland-utils mesa libGLU libGL alsa-lib pulseaudio];
        sconsFlags = mkSconsFlagsFromAttrSet {
          target = "editor";
          platform = "linux";
          arch = "x86_64";
          compiledb="yes";
        };
        src = godot-cpp;
        buildPhase = ''
        scons compiledb=yes
        '';
        installPhase = ''
        ls -a
        cp -rT . $out
        '';
      };
      steamworks-sdk = with stdenv; mkDerivation rec {
        pname = "steamworks-sdk";
        version = "1.6.1";
        nativeBuildInputs = with pkgs; [unzip];
        src = pkgs.requireFile {
          name = "steamworks_sdk_161.zip";
          url = "https://partner.steamgames.com/downloads/steamworks_sdk_161.zip";
          sha256 = "970eb85823fb4bba7bcd2f1cc99b347a2b3f9c985bd1f3a9c73018e277057450";
        };
        unpackPhase = ''
        unzip $src -d $out
        '';
      };
      godot-multiplayer-peer = with stdenv; mkDerivation rec {
        pname = "godot-multiplayer-peer";
        version = "0.2.3";
        src = ./.;
        nativeBuildInputs = with pkgs; [scons self.packages.${system}.godot-cpp self.packages.${system}.steamworks-sdk];
        buildPhase = ''
          cp -rT ${self.packages.${system}.godot-cpp} godot-cpp
          rm -rf steam-multiplayer-peer/sdk
          ln -sT ${self.packages.${system}.steamworks-sdk}/sdk steam-multiplayer-peer/sdk 
          # note to self, figure out way to use godot-cpp package instead of compiling in place
          chmod -R 755 godot-cpp
          echo "."
          ls -la
          echo "core"
          ls -la godot-cpp/gen/include/godot_cpp/core
          ls -la steam-multiplayer-peer/sdk
          scons debug_symbols=yes compiledb=yes
        '';
        installPhase = ''
        cp -r bin/ $out
        cp LICENSE $out
        cp README.md $out
        cp steam-multiplayer-peer.gdextension $out
        cp compile_commands.json $out
        '';
      };
    };
  };
}
