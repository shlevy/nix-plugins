{
  description = "Nix plugins for extra builtins";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
  };

  outputs = { self, nixpkgs }: let
    systems = [ "x86_64-linux" "aarch64-linux" "x86_64-darwin" "aarch64-darwin" ];
    forAllSystems = nixpkgs.lib.genAttrs systems;
  in {
    devShells = forAllSystems (system: {
      default = nixpkgs.legacyPackages.${system}.mkShell {
        buildInputs = with nixpkgs.legacyPackages.${system}; [
          cmake
          pkg-config
          nixVersions.nixComponents_2_30.nix-store
          nixVersions.nixComponents_2_30.nix-expr
          nixVersions.nixComponents_2_30.nix-cmd
          nixVersions.nixComponents_2_30.nix-fetchers
          boost
        ];
      };
    });
  };
}
