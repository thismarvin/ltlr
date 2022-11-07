{
  description = "a 2D action platformer";

  inputs.nixpkgs.url = "github:nixos/nixpkgs";

  outputs = { self, nixpkgs }:
    let
      pname = "ltlr";
      version = "0.1.0";
      system = "x86_64-linux";
      pkgs = import nixpkgs { inherit system; };
    in
    {
      packages."${system}" = {
        ltlr = derivation {
          name = "${pname}-${version}";
          inherit pname;
          inherit version;
          src = ./.;
          inherit system;
          builder = "${pkgs.nushell}/bin/nu";
          args = [ ./builder.nu ];
          buildInputs = with pkgs; [
            xorg.libX11.dev
            xorg.libXcursor.dev
            xorg.libXext.dev
            xorg.libXfixes.dev
            xorg.libXi.dev
            xorg.libXinerama.dev
            xorg.libXrandr.dev
            xorg.libXrender.dev
            xorg.xorgproto
            libGL.dev
            coreutils
            binutils-unwrapped
            nushell
            pkg-config
            gcc
            patchelf
            gnumake
          ];
        };
        default = self.packages."${system}".ltlr;
      };

      devShells."${system}" = {
        default = pkgs.mkShell {
          inputsFrom = [ self.packages."${system}".default ];
          packages = with pkgs; [
            astyle
            nodePackages.prettier
            emscripten
          ];
        };
      };
    };
}
