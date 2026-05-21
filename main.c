#include <SDL2/SDL.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_BULLETS 100

typedef struct {
    float  x, y;
    char *name;
    int healthBar;
    int movementSpeed;
} Character;

typedef struct {
    int x , y;
    int isDestroyed;
} Target;

typedef struct {
    float x, y;
    float speed;
    int active;
    float directionx;
    float directiony;
} Bullet;

int getRandomNumber(int min , int max){

    return min + rand() % (max - min + 1);
}

void drawGrid(SDL_Renderer* renderer, int width, int height, int cellSize) {

    // Set grid color (gray)
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);

    // Vertical lines
    for (int x = 0; x <= width; x += cellSize) {
        SDL_RenderDrawLine(renderer, x, 0, x, height);
    }

    // Horizontal lines
    for (int y = 0; y <= height; y += cellSize) {
        SDL_RenderDrawLine(renderer, 0, y, width, y);
    }
}

void bulletHitCheck(Target *target, Bullet bullets[MAX_BULLETS]){

    SDL_Rect tar = {(int)target->x, (int)target->y, 50, 50};
    for(int i = 0; i < MAX_BULLETS; i++){
        SDL_Rect bul  = {(int)bullets[i].x, (int)bullets[i].y, 10, 10};
        if (SDL_HasIntersection(&bul, &tar)) {
            bullets[i].active = 0;
            target->x = getRandomNumber(0 , 15) * 50;
            target->y = getRandomNumber(0 , 11) * 50;
        }
    }
}

void handleInput(Character *player, int *running, Bullet bullets[MAX_BULLETS]) {

    // Handle events
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            *running = 0;
            break;
        }
        
        int mousex, mousey;

        SDL_GetMouseState(&mousex, &mousey);

        float dx = mousex - player->x;
        float dy = mousey - player->y;

        float length = sqrt(dx * dx + dy * dy);

        dx /= length;
        dy /= length;

        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) {
                *running = 0;
                break;
            }
            if (event.key.keysym.sym == SDLK_SPACE) {
                // find inactive bullet
                for (int i = 0; i < MAX_BULLETS; i++) {
                    if (!bullets[i].active) {
                        bullets[i].x = player->x;
                        bullets[i].y = player->y;
                        
                        bullets[i].directionx = dx;
                        bullets[i].directiony = dy;

                        bullets[i].speed = 500;
                        bullets[i].active = 1;
                        break;
                    }
                }
            }
        }   
    }
}

void update(float deltaTime, Character *player, Target *target, Bullet bullets[]) {
    
    // check keyboard input
    const Uint8* state = SDL_GetKeyboardState(NULL);
    if (state[SDL_SCANCODE_W])  player->y -= player->movementSpeed * deltaTime;
    if (state[SDL_SCANCODE_S])  player->y += player->movementSpeed * deltaTime;
    if (state[SDL_SCANCODE_A])  player->x -= player->movementSpeed * deltaTime;
    if (state[SDL_SCANCODE_D])  player->x += player->movementSpeed * deltaTime;

    // initialize 
    SDL_Rect rect = {(int)player->x, (int)player->y, 50, 50};
    SDL_Rect tar  = {target->x, target->y, 50, 50};

    // check bullet collision
    bulletHitCheck(target , bullets);

    // check collision 
    if (SDL_HasIntersection(&rect, &tar)) {
        target->x = getRandomNumber(0 , 15) * 50;
        target->y = getRandomNumber(0 , 11) * 50;
    }

    for (int i = 0; i < MAX_BULLETS; i++) {
        if (bullets[i].active) {
            
            bullets[i].x += bullets[i].directionx * bullets[i].speed * deltaTime;
            bullets[i].y += bullets[i].directiony * bullets[i].speed * deltaTime;
            
            // remove if off screen
            if (bullets[i].y < 0 || bullets[i].y > 600 || bullets[i].x < 0 || bullets[i].x > 800) {
                bullets[i].active = 0;         
            }
        }
    }
}

void doRendering(SDL_Renderer *renderer , Character *player, Target *target , Bullet bullets[]) {
    
    // Clear the screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // black background
    SDL_RenderClear(renderer);
    
    // draw grid first (background)
    drawGrid(renderer, 800, 600, 50);
    
    // Render the player character here (e.g., as a rectangle)
    SDL_Rect rect = {(int)player->x, (int)player->y, 50, 50};
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // green color for the player
    SDL_RenderFillRect(renderer, &rect);

    // Draw the target
    SDL_Rect tar = {target->x , target->y , 50, 50};  
    SDL_SetRenderDrawColor(renderer, 255, 0 , 0 , 255);
    SDL_RenderFillRect(renderer, &tar);

    // Draw bullets
    for(int i = 0; i < MAX_BULLETS; i++){
        if(bullets[i].active){
            SDL_Rect bul = {(int)bullets[i].x , (int)bullets[i].y , 10, 10};  
            SDL_SetRenderDrawColor(renderer, 0, 0 , 255 , 255);
            SDL_RenderFillRect(renderer, &bul);
        }
    }

    // Render the player character here (e.g., as a rectangle)
    SDL_RenderPresent(renderer);
}

float calculateDeltaTime(Uint32 *lastTime) {
    Uint32 currentTime = SDL_GetTicks();
    float deltaTime = (currentTime - *lastTime) / 1000.0f; // Convert to seconds
    *lastTime = currentTime;
    return deltaTime;
}

int main(int argc, char* argv[]) {

    // seed 
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("SDL Init Error: %s", SDL_GetError());
        return 1;
    }

    // Create a window
    SDL_Window *window = SDL_CreateWindow("Hello SDL",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN);
    
    // Create a renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    // Initialize the player character
    Character player = {0, 0, "Player", 100, 500};

    // Initialize the target
    Target target = {getRandomNumber(0 , 15) * 50, getRandomNumber(0 , 15) * 50 , 0 };

    // Initialize bullets
    Bullet bullets[MAX_BULLETS];

    // Main game loop
    
    int running = 1;

    // Timing variables for frame rate control
    Uint32 lastTime = SDL_GetTicks();

    for (int i = 0; i < MAX_BULLETS; i++) {
        bullets[i].active = 0;
    }

    while (running) {
        
        // Handle input, update game state, and render
        handleInput(&player, &running, bullets);
        
        // update player position and deltaTime
        update(calculateDeltaTime(&lastTime), &player , &target , bullets);

        // Render game
        doRendering(renderer, &player, &target , bullets);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}