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
        version = "2023-02-06";
        src = ./.;
        nativeBuildInputs = with pkgs; [
          makeWrapper
          nushell
          pkg-config
        ];
        buildInputs = with pkgs; [
          xorg.libX11
          xorg.libXrandr
          xorg.libXinerama
          xorg.libXcursor
          xorg.libXi
          xorg.libXext
          xorg.libXfixes
          xorg.libXrender
          xorg.libxcb
          xorg.libXau
          xorg.libXdmcp
          glfw
        ];
        buildPhase = ''
          nu $src/scripts/build_phase.nu
        '';
        installPhase = ''
          mkdir -p $out/bin
          cp -r $TMP/content $out/bin
          cp $TMP/ltlr $out/bin
          wrapProgram $out/bin/ltlr --chdir $out/bin
        '';
      };
      default = self.packages."${system}".ltlr;
    };
    devShells."${system}" = {
      minimal = pkgs.mkShell {
        inputsFrom = [self.packages."${system}".default];
        packages = with pkgs; [
          clang-tools
        ];
      };
      extra = pkgs.mkShell {
        inputsFrom = [self.devShells."${system}".minimal];
        packages = with pkgs; [
          emscripten
        ];
      };
      default = self.devShells."${system}".minimal;
    };
  };
}
