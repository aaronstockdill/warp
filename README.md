# Warp

Jumping through your filesystem at warp speed.

## Usage

Basic usage: just run warp/warptool. The help will be printed.

Setting a warp point is simple. `cd` to the directory you want to make a warp point, then run `warp set foobar`, where `foobar` is the name of the warp point.

To return to a warp point in the future, run `warp jump foobar`.

## Building

    make warptool

## Installing

Move `bin/warptool` to somewhere in your path. Then insert the `warp` function in `src/warp.sh` into your `.bashrc`/`.zshrc`/whatever. The `warp` function relies on fzf. If you don't have fzf, install it! You're missing out!

## Todos:

- Test in bash, etc.
- Test on anything that's not macOS.
- Have a non-fzf version.
- Proper install scripts.
