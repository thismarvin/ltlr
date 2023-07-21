# Larry the Light-bulb Redux

Revisiting Larry the Light-bulb

## How to Play

You can play `ltlr` online at [https://ltlr.delveam.com][0].

### Controls

- `Arrow Keys` + `Z` + `X`
- `WASD` + `Space` + `J`
- You can also use a Gamepad!

### PWA

It's worth noting that `ltlr` is a [progressive web app][1], so you can even install it and play it offline!

## Building from Source

### Prerequisites

- [Nix: the package manager][2]
    - Make sure [flakes][3] are enabled.

### Installation

Running `ltlr` is as simple as follows:

#### On NixOS

```bash
nix run github:delveam/ltlr
```

#### On other Distros

Using Nix + OpenGL on other distros is awkward; you're forced to use [nixGL][4].

##### Mesa OpenGL Implementation

```bash
nix \
    run \
    --inputs-from github:delveam/ltlr \
    github:guibou/nixGL#nixGLIntel \
    -- \
        nix run github:delveam/ltlr
```

##### Proprietary Nvidia Driver

```bash
nix \
    run \
    --inputs-from github:delveam/ltlr \
    github:guibou/nixGL#nixGLNvidia \
    -- \
        nix run github:delveam/ltlr
```

[0]: https://ltlr.delveam.com
[1]: https://developer.mozilla.org/en-US/docs/Web/Progressive_web_apps
[2]: https://nixos.org/download.html#nix-install-linux
[3]: https://nixos.wiki/wiki/Flakes#Permanent
[4]: https://github.com/guibou/nixGL
