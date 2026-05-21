#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

#define PLAYER_WIDTH 200
#define PLAYER_HEIGHT 200

#define MONSTER_WIDTH 120
#define MONSTER_HEIGHT 120

#define FRAME_WIDTH 1000
#define FRAME_HEIGHT 1154

#define PLAYER_SPEED 300.0f

typedef struct {
    float x, y;
    int frame;
    SDL_Texture* texture;
} Player;

typedef struct {
    SDL_Rect rect;
    SDL_Texture* texture;
} Monster;

typedef struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    SDL_Event event;

    int running;
    int hit_played;

    Player player;
    Monster monster;

    Mix_Chunk* hit_sound;
} State;

Uint32 UpdateAnimation(Uint32 interval, void* param) {
    State* state = (State*)param;
    state->player.frame = (state->player.frame + 1) % 10;
    return interval;
}

SDL_Texture* LoadTexture(State* state, const char* path) {
    SDL_Surface* surface = IMG_Load(path);

    if(surface == NULL) {
        printf("Failed to load %s\n", path);
        exit(1);
    }

    SDL_Texture* texture =
        SDL_CreateTextureFromSurface(state->renderer, surface);

    SDL_FreeSurface(surface);

    return texture;
}

void Initialize(State* state) {

    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    state->window = SDL_CreateWindow(
        "SDL2 Game",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        0
    );

    state->renderer = SDL_CreateRenderer(
        state->window,
        -1,
        SDL_RENDERER_ACCELERATED
    );

    state->running = 1;
    state->hit_played = 0;

    // Player

    state->player.x = 100;
    state->player.y = 100;
    state->player.frame = 0;

    state->player.texture =
        LoadTexture(state, "mycharacter.png");

    // Monster

    state->monster.rect.x = 500;
    state->monster.rect.y = 250;
    state->monster.rect.w = MONSTER_WIDTH;
    state->monster.rect.h = MONSTER_HEIGHT;

    state->monster.texture =
        LoadTexture(state, "monster.png");

    // Sound

    state->hit_sound = Mix_LoadWAV("fahh.wav");

    if(state->hit_sound == NULL) {
        printf("Failed to load sound\n");
    }
}

void HandleEvents(State* state) {

    while(SDL_PollEvent(&state->event)) {

        if(state->event.type == SDL_QUIT) {
            state->running = 0;
        }
    }
}

void HandleInput(State* state, float delta_time) {

    const Uint8* keystate =
        SDL_GetKeyboardState(NULL);

    if(keystate[SDL_SCANCODE_W]) {
        state->player.y -= PLAYER_SPEED * delta_time;
    }

    if(keystate[SDL_SCANCODE_S]) {
        state->player.y += PLAYER_SPEED * delta_time;
    }

    if(keystate[SDL_SCANCODE_A]) {
        state->player.x -= PLAYER_SPEED * delta_time;
    }

    if(keystate[SDL_SCANCODE_D]) {
        state->player.x += PLAYER_SPEED * delta_time;
    }
}

void Update(State* state) {

    // Window bounds

    if(state->player.x < 0)
        state->player.x = 0;

    if(state->player.y < 0)
        state->player.y = 0;

    if(state->player.x > WINDOW_WIDTH - PLAYER_WIDTH)
        state->player.x = WINDOW_WIDTH - PLAYER_WIDTH;

    if(state->player.y > WINDOW_HEIGHT - PLAYER_HEIGHT)
        state->player.y = WINDOW_HEIGHT - PLAYER_HEIGHT;

    // Collision

    SDL_Rect player_rect = {
        (int)state->player.x,
        (int)state->player.y,
        PLAYER_WIDTH,
        PLAYER_HEIGHT
    };

    if(SDL_HasIntersection(
        &player_rect,
        &state->monster.rect
    )) {

        if(!state->hit_played) {

            Mix_PlayChannel(
                -1,
                state->hit_sound,
                0
            );

            state->hit_played = 1;
        }

    } else {

        state->hit_played = 0;
    }
}

void RenderPlayer(State* state) {

    SDL_Rect src_rect;

    src_rect.x =
        (state->player.frame % 5) * FRAME_WIDTH;

    src_rect.y =
        (state->player.frame / 5) * FRAME_HEIGHT;

    src_rect.w = FRAME_WIDTH;
    src_rect.h = FRAME_HEIGHT;

    SDL_Rect dst_rect = {
        (int)state->player.x,
        (int)state->player.y,
        PLAYER_WIDTH,
        PLAYER_HEIGHT
    };

    SDL_RenderCopy(
        state->renderer,
        state->player.texture,
        &src_rect,
        &dst_rect
    );
}

void RenderMonster(State* state) {

    SDL_RenderCopy(
        state->renderer,
        state->monster.texture,
        NULL,
        &state->monster.rect
    );
}

void Render(State* state) {

    SDL_SetRenderDrawColor(
        state->renderer,
        25,
        25,
        25,
        255
    );

    SDL_RenderClear(state->renderer);

    RenderPlayer(state);

    RenderMonster(state);

    SDL_RenderPresent(state->renderer);
}

void Cleanup(State* state) {

    SDL_DestroyTexture(state->player.texture);
    SDL_DestroyTexture(state->monster.texture);

    Mix_FreeChunk(state->hit_sound);

    SDL_DestroyRenderer(state->renderer);
    SDL_DestroyWindow(state->window);

    Mix_CloseAudio();

    IMG_Quit();
    SDL_Quit();
}

int main(int argc, char* argv[]) {

    State state;

    Initialize(&state);

    SDL_AddTimer(
        150,
        UpdateAnimation,
        &state
    );

    Uint32 last_time = SDL_GetTicks();

    while(state.running) {

        Uint32 current_time = SDL_GetTicks();

        float delta_time =
            (current_time - last_time) / 1000.0f;

        last_time = current_time;

        HandleEvents(&state);

        HandleInput(&state, delta_time);

        Update(&state);

        Render(&state);

        SDL_Delay(1);
    }

    Cleanup(&state);

    return 0;
}
