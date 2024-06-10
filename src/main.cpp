#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>
#include <filesystem>
#include <deque>

namespace fs = std::filesystem;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int MAX_SCREEN_WIDTH = 2560;
const int MAX_SCREEN_HEIGHT = 1440;
const int PADDING = 200;
const int INITIAL_SIZE = 2, INITIAL_SPEED = 200;
const int FPS_CAP = 30;

std::vector<std::vector<bool>> board;
std::deque<std::pair<int, int>> snake;

int random(int min, int max)
{
    return rand() % (max - min + 1) + min;
}

enum Direction
{
    UP,
    DOWN,
    LEFT,
    RIGHT
};

SDL_Color White = {255, 255, 255}, Black = {0, 0, 0}, Red = {255, 0, 0}, Green = {0, 255, 0}, Yellow = {255, 255, 0};

void drawGrid(SDL_Renderer *renderer, int width, int height, int boardSize, std::vector<std::vector<bool>> board, int headX, int headY, int foodX, int foodY)
{
    // border of the whole grid
    int rectSize = boardSize / board.size();

    SDL_Rect rect = {width - 2, height - 2, rectSize * board.size() + 4, rectSize * board.size() + 4};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);

    int startPointW = width;
    int startPointH = height + rectSize * (board.size() - 1);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderDrawRect(renderer, &rect);

    for (int x = 0; x < board.size(); x++)
    {
        for (int y = 0; y < board[x].size(); y++)
        {
            // std::cout << x << ' ' << y << std::endl;
            rect = {startPointW + x * rectSize, startPointH - y * rectSize, rectSize, rectSize};
            if (board[y][x])
            {
                if (headX == x && headY == y)
                {
                    SDL_SetRenderDrawColor(renderer, 0, Yellow.r, Yellow.g, Yellow.b);
                }
                else
                {
                    SDL_SetRenderDrawColor(renderer, Red.r, Red.g, Red.b, 255);
                }
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            }

            if (foodX == x && foodY == y)
            {
                SDL_SetRenderDrawColor(renderer, Green.r, Green.g, Green.b, 255);
            }

            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

int main(int argv, char **args)
{
    // initialize SDL
    bool fullscreen = false;
    int currentWidth = SCREEN_WIDTH, currentHeight = SCREEN_HEIGHT, fps;
    SDL_Color Black = {0, 0, 0};

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();

    TTF_Font *InterBold = TTF_OpenFont(".\\Inter-Bold.ttf", 64);
    if (InterBold == NULL)
    {
        std::cout << "Font Inter failed to load." << std::endl;
        std::cout << TTF_GetError() << std::endl;
        return 3;
    }

    SDL_Window *window = SDL_CreateWindow("The Snake Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN);

    if (window == NULL)
    {
        std::cout << "Window failed to create." << std::endl;
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    if (renderer == NULL)
    {
        std::cout << "Renderer failed to create." << std::endl;
        return 2;
    }

    // initialize game
    int score = 0, size = INITIAL_SIZE, boardSize = 20, direction = RIGHT, moveDelay = INITIAL_SPEED, lastMove = 0, screenBoardSize, lastTick = 0, foodX = random(0, boardSize - 1), foodY = random(0, boardSize - 1);
    int x = 0, y = boardSize / 2;

    board = std::vector<std::vector<bool>>(boardSize, std::vector<bool>(boardSize, false));

    // main loop
    SDL_Event e;

    bool quit = false, gameOver = false, scoreChanged = true;
    SDL_Surface *surfaceMessage;
    SDL_Texture *Message;

    while (!quit)
    {
        if (SDL_GetTicks() - lastTick < 1000 / FPS_CAP)
        {
            SDL_Delay(1000 / FPS_CAP - (SDL_GetTicks() - lastTick));
            continue;
        }
        SDL_GetWindowSize(window, &currentWidth, &currentHeight);
        while (SDL_PollEvent(&e) != 0)
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }

            if (e.type == SDL_KEYDOWN)
            {
                switch (e.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                case SDLK_F11:
                    if (fullscreen)
                    {
                        SDL_SetWindowFullscreen(window, 0);
                        SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
                        fullscreen = false;
                        currentHeight = SCREEN_HEIGHT;
                        currentWidth = SCREEN_WIDTH;
                    }
                    else
                    {
                        SDL_SetWindowSize(window, MAX_SCREEN_WIDTH, MAX_SCREEN_HEIGHT);
                        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
                        fullscreen = true;
                        currentHeight = MAX_SCREEN_HEIGHT;
                        currentWidth = MAX_SCREEN_WIDTH;
                    }
                    break;

                case SDLK_UP:
                    if (direction != DOWN)
                        direction = UP;
                    break;
                case SDLK_DOWN:
                    if (direction != UP)
                        direction = DOWN;
                    break;
                case SDLK_LEFT:
                    if (direction != RIGHT)
                        direction = LEFT;
                    break;
                case SDLK_RIGHT:
                    if (direction != LEFT)
                        direction = RIGHT;
                    break;

                case SDLK_SPACE:
                    gameOver = true;
                    break;

                case SDLK_w:
                    if (direction != DOWN)
                        direction = UP;
                    break;

                case SDLK_s:
                    if (direction != UP)
                        direction = DOWN;
                    break;

                case SDLK_a:
                    if (direction != RIGHT)
                        direction = LEFT;
                    break;

                case SDLK_d:
                    if (direction != LEFT)
                        direction = RIGHT;
                    break;

                default:
                    break;
                }
            }
        }

        if (gameOver)
        {
            // reset game
            gameOver = false;
            score = 0;
            size = INITIAL_SIZE;
            direction = RIGHT;
            moveDelay = INITIAL_SPEED;
            lastMove = SDL_GetTicks();
            x = 0;
            y = boardSize / 2;
            snake.clear();
            snake.push_front(std::make_pair(x, y));
            board = std::vector<std::vector<bool>>(boardSize, std::vector<bool>(boardSize, false));
            scoreChanged = true;
        }

        if (SDL_GetTicks() - lastMove > moveDelay)
        {
            lastMove = SDL_GetTicks();
            switch (direction)
            {
            case UP:
                if (y == boardSize - 1)
                {
                    y = 0;
                }
                else
                {
                    y++;
                }
                break;
            case DOWN:
                if (y == 0)
                {
                    y = boardSize - 1;
                }
                else
                {
                    y--;
                }
                break;
            case LEFT:
                if (x == 0)
                {
                    x = boardSize - 1;
                }
                else
                {
                    x--;
                }
                break;
            case RIGHT:
                if (x == boardSize - 1)
                {
                    x = 0;
                }
                else
                {
                    x++;
                }
                break;
            default:
                break;
            }

            snake.push_front(std::make_pair(x, y));

            if (x == foodX && y == foodY)
            {
                size++;
                score++;
                moveDelay -= 2;
                foodX = random(0, boardSize - 1);
                foodY = random(0, boardSize - 1);
                while (board[foodY][foodX])
                {
                    foodX = random(0, boardSize - 1);
                    foodY = random(0, boardSize - 1);
                }
                scoreChanged = true;
            }

            if (snake.size() > size)
            {
                board[snake.back().second][snake.back().first] = false;
                snake.pop_back();
            }

            if (board[y][x])
            {
                gameOver = true;
            }

            board[y][x] = true;
        }

        surfaceMessage =
            TTF_RenderText_Solid(InterBold, std::to_string(score).c_str(), Black);

        Message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);

        SDL_Rect Message_rect;
        Message_rect.x = 20; // get the center of the screen
        Message_rect.y = 10;
        Message_rect.w = // the width of the text
            surfaceMessage->w;
        Message_rect.h = // the height of the text
            surfaceMessage->h;

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 0);
        SDL_RenderClear(renderer);
        screenBoardSize = currentHeight * 0.8;
        SDL_RenderCopy(renderer, Message, NULL, &Message_rect);

        drawGrid(renderer, currentWidth / 2 - screenBoardSize / 2, currentHeight / 2 - screenBoardSize / 2, screenBoardSize, board, snake.front().first, snake.front().second, foodX, foodY);

        SDL_RenderPresent(renderer);

        SDL_DestroyTexture(Message);
        SDL_FreeSurface(surfaceMessage);

        lastTick = SDL_GetTicks();
    }

    TTF_Quit();
    SDL_Quit();
    return 0;
}