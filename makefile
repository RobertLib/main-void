all:
	cc main.c `pkg-config --libs --cflags raylib` # -g -fsanitize=address
