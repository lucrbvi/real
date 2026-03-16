# Real

Real, also known as the opposite of Unreal, is a simple game framework based on [raylib](https://raylib.com) and [mruby](https://mruby.org/).

## Get started

The only dependency you must install on your computer is Zig. Click [here](https://ziglang.org/learn/getting-started/) to install Zig.

Thanks to Zig, raylib will be built on your own machine, you don't have to manually install it!

We have an amalgamated version of mruby in the repo, so you don't have to install it on your machine (is also built on your machine).

Next, you can run `zig build` to produce artifacts, `zig build run` to play your game and `zig build dev` which does not recompile your ruby code (save a tone of time!).

Now you can edit `game/main.rb` and start building your game!

*Important:* Cross-compilation does not work well yet (at least on my machine), because (1) mruby's C code does not respect Windows (generate a LOT of errors) and (2) you need to have a tone of libraries installed on your machine to link to produce the binary.

## Documentation

We provide a beautifull documentation throught [RDoc](https://ruby.github.io/rdoc/), we just need to write in Ruby files the declarations for our classes and methods.

To generate the documentation you must run `rdoc --main README.md --markup markdown README.md engine/real.rb --format darkfish`.
Then you can spin up a server to see the doc in a web browser by running `ruby -run -e httpd doc/ -p 8080` or `python3 -m http.server 8080 --directory doc/`.

*Important:* You may see that the source of methods and classes shown by RDoc is empty, because we implement our code in C, but we use a Ruby file to write the documentation! If you are curious about the implementation, please read `./engine/real.c`!
