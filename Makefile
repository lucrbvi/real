CC = gcc
CFLAGS = -Wall -I./engine/libs
LDFLAGS = -L./engine/libs -lraylib -lm -lpthread -lX11

out/real: engine/real.c engine/libs/mruby.h engine/libs/mruby.c engine/libs/raylib.h
	$(CC) $(CFLAGS) engine/real.c engine/libs/mruby.c -o out/real $(LDFLAGS)

out/game: engine/real.c engine/libs/mruby.h engine/libs/mruby.c engine/libs/raylib.h engine/game.h
	$(CC) $(CFLAGS) -DINCLUDE_GAME engine/real.c engine/libs/mruby.c -o out/game $(LDFLAGS)

engine/game.h: out/real game/main.rb
	./out/real compile game/main.rb game/main.mrb
	xxd -i game/main.mrb > engine/game.h

run: out/game
	./out/game
