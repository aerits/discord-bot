{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";
    utils.url = "github:numtide/flake-utils";
  };

  outputs =
    {
      self,
      nixpkgs,
      utils,
    }:
    utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
        deps = with pkgs; [
          clang-tools
          nix-index
          cmake
          libcxx
          dpp
          python3
          pkg-config
          libmpg123
          xmake
          nlohmann_json
          zlib
        ];
      in
      {
        devShell =
          with pkgs;
          mkShell.override { stdenv = clangStdenv; } {
            buildInputs = deps;
            nativeBuildInputs = deps;
          };
      }
    );
}
