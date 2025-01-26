{
  description = "a 2D action platformer";

  inputs.nixpkgs.url = "github:nixos/nixpkgs/nixpkgs-unstable";

  outputs = {
    self,
    nixpkgs,
  }: {
    formatter = {
      x86_64-linux = nixpkgs.legacyPackages.x86_64-linux.alejandra;
    };
    packages = {
      x86_64-linux = let
        pkgs = nixpkgs.legacyPackages.x86_64-linux;
      in {
        ltlr = let
          fs = pkgs.lib.fileset;
          sourceFiles = fs.intersection (fs.gitTracked ./.) (
            fs.unions [
              ./content
              ./src
              ./vendor
              ./Build.mk
              ./Common.mk
              ./Desktop.mk
              ./Makefile
            ]
          );
        in
          pkgs.stdenv.mkDerivation {
            pname = "ltlr";
            version = "2025-01-26";
            src = fs.toSource {
              root = ./.;
              fileset = sourceFiles;
            };
            nativeBuildInputs = with pkgs; [
              pkg-config
            ];
            buildInputs = with pkgs; [
              glfw
            ];
            enableParallelBuilding = true;
            makeFlags = ["prefix=$(out)"];
          };
        default = self.packages.x86_64-linux.ltlr;
      };
    };
    devShells = {
      x86_64-linux = let
        pkgs = nixpkgs.legacyPackages.x86_64-linux;
      in {
        minimal = pkgs.mkShell {
          inputsFrom = [self.packages.x86_64-linux.default];
          packages = with pkgs; [
            clang-tools
            nushell
          ];
        };
        extra = pkgs.mkShell {
          inputsFrom = [self.devShells.x86_64-linux.minimal];
          packages = with pkgs; [
            emscripten
            zig
          ];
        };
        default = self.devShells.x86_64-linux.minimal;
      };
    };
  };
}
