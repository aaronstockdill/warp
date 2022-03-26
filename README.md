# Warp

Jumping through your filesystem at warp speed.

## Usage

Basic usage: just run `warp`. The help will be printed.

Help is also printed with `warp help`, and `warp help [command]` gives more detail about each of the commands below. Alternatively, run `warp [command] -h`. (Basically, there's lots of ways to show the help, meaning you should stumble upon it pretty easily.)

Setting a warp point is simple enough. `cd` to the directory you want to make a warp point, then run `warp set foobar`, where `foobar` is the name of the warp point. Alternatively, you can run `warp set foobar path/to/dir` to set a path without actually having to be there. If the warp point is already in use, this fails; use `warp set --force foobar new/path/to/dir` to overwrite the old warp point.

To return to a warp point in the future, run `warp jump foobar`. (Personally, I alias `warp jump` to `j`.)

To see all the warp points, run `warp list`; to see them all with their paths, run `warp list -p`. For just the path for `foobar`, use `warp dir foobar`.

If you no longer need a particular warp point, run `warp remove foobar`. To "remove" warp points that don't exist without raising a fuss, run `warp remove foobaz --quiet`.

## Building

    make warptool

## Installing

Move `bin/warptool` to somewhere in your path. Then somehow insert the `warp` function in `src/warp.sh` into your `.bashrc`/`.zshrc`/whatever. The `warp` function relies on fzf. If you don't have fzf, install it! You're missing out!

## Todos:

- Test in bash, etc.
- Test on anything that's not macOS.
- Proper install scripts.
