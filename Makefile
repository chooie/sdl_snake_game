all:
	g++ -Wall -Werror -Wconversion -I vendor/SDL2/include -L vendor/SDL2/lib -o main main.cpp -lmingw32 -lSDL2main -lSDL2