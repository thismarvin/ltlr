{
  description = "a 2D action platformer";

  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";

  outputs = {
    self,
    nixpkgs,
  }: let
    system = "x86_64-linux";
    pkgs = import nixpkgs {inherit system;};
  in {
    formatter."${system}" = pkgs.alejandra;
    packages."${system}" = {
      ltlr = pkgs.stdenv.mkDerivation {
        pname = "ltlr";
        version = "2023-02-13";
        src = ./.;
        nativeBuildInputs = with pkgs; [
          pkg-config
        ];
        buildInputs = with pkgs; [
          xorg.libXau
          xorg.libXdmcp
          xorg.libxcb
          xorg.libXext
          xorg.libXrender
          xorg.libXfixes
          xorg.libX11
          xorg.libXrandr
          xorg.libXinerama
          xorg.libXcursor
          xorg.libXi
          glfw
        ];
        enableParallelBuilding = true;
        makeFlags = ["prefix=$(out)"];
      };
      default = self.packages."${system}".ltlr;
    };
    devShells."${system}" = {
      minimal = pkgs.mkShell {
        inputsFrom = [self.packages."${system}".default];
        packages = with pkgs; [
          clang-tools
          nushell
        ];
      };
      extra = pkgs.mkShell {
        inputsFrom = [self.devShells."${system}".minimal];
        packages = with pkgs; [
          emscripten
          zig
        ];
      };
      default = self.devShells."${system}".minimal;
    };
  };
}
