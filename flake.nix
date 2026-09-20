{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    systems.url = "github:nix-systems/default-linux";
  };

  outputs = {
    nixpkgs,
    systems,
    ...
  }: let
    forEachSystem = callback:
      nixpkgs.lib.genAttrs (import systems) (
        system: callback nixpkgs.legacyPackages.${system}
      );
  in {
    packages = forEachSystem (
      pkgs: let
        package = pkgs.callPackage ./scripts/nix/package.nix {};
      in {
        default = package;
        ntfyDesktop = package;
      }
    );
  };
}
