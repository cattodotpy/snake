all:
	g++ src/main.cpp -I SDL2/include -L SDL2/lib -l mingw32 -l SDL2main -l SDL2 -l SDL2_ttf -o build/snake.exe -mwindows