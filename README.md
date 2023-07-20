# Larry the Light-bulb Redux

Revisiting Larry the Light-bulb

You can play `ltlr` online at [https://ltlr.delveam.com][0]. `ltlr` is a progressive web app, so you can even download it and play it offline!

## Building from Source

### Prerequisites

- [Nix: the package manager][1]

### Installation

Running `ltlr` is as simple as follows:

#### On NixOS

```bash
nix run github:delveam/ltlr
```

#### On other Distros

Using Nix + OpenGL is awkward; luckily, [nixGL][2] exists!

##### Mesa OpenGL Implementation

```bash
nix run --inputs-from . github:guibou/nixGL#nixGLIntel -- nix run github:delveam/ltlr
```

##### Proprietary Nvidia Driver

```bash
nix run --inputs-from . github:guibou/nixGL#nixGLNvidia -- nix run github:delveam/ltlr
```

[0]: https://ltlr.delveam.com
[1]: https://nixos.org/download.html#nix-install-linux
[2]: https://github.com/guibou/nixGL
