/**
* Author: Ninad Moharir
* Assignment: Rise of the AI
* Date due: 2024-11-03, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/


#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define ENEMY_COUNT 1
#define LEVEL1_WIDTH 38
#define LEVEL1_HEIGHT 5

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"

//  GAME STATE  //
struct GameState
{
    Entity* player;
    Entity* enemies;

    Map* map;

    Mix_Music* bgm;
    Mix_Chunk* jump_sfx;
};

enum AppStatus { RUNNING, TERMINATED, WON, LOST };

//  CONSTANTS  //
constexpr int WINDOW_WIDTH = 640 * 1.5,
WINDOW_HEIGHT = 480 * 1.5;

std::string text_to_display = "";


constexpr float BG_RED = 0.1922f,
BG_BLUE = 0.549f,
BG_GREEN = 0.9059f,
BG_OPACITY = 1.0f;

constexpr int VIEWPORT_X = 0,
VIEWPORT_Y = 0,
VIEWPORT_WIDTH = WINDOW_WIDTH,
VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char GAME_WINDOW_NAME[] = "matrix shooter";

constexpr char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr char SPRITESHEET_FILEPATH[] = "assets/neo.png",
FONT_FILEPATH[] = "assets/font1.png",
ENEMY_FILEPATH[] = "assets/smith.png",
MAP_TILESET_FILEPATH[] = "assets/Textures-16.png",
BGM_FILEPATH[] = "assets/audio/dooblydoo.mp3",
JUMP_SFX_FILEPATH[] = "assets/audio/bounce.wav";

constexpr int NUMBER_OF_TEXTURES = 1;
constexpr GLint LEVEL_OF_DETAIL = 0;
constexpr GLint TEXTURE_BORDER = 0;

float g_time_left = 15.0f;
int g_enemies_left = 3;

unsigned int LEVEL_1_DATA[] =
{
    /*0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,*/
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 3,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 3, 4, 1, 3, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 3, 2, 0, 0, 0, 1, 0, 0, 0, 3,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 2, 2, 2, 2, 0, 0, 2, 1, 2, 2, 2, 2, 2, 0, 0, 0, 3,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 3, 2, 0, 0, 0, 1, 0, 0, 0, 3,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 1, 1, 0, 4, 1, 1, 1, 2, 2, 3, 0, 0, 2, 2, 2, 2, 0, 0, 2, 2, 2, 2, 0, 0, 0, 3
};

//  VARIABLES  //
GameState g_game_state;

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program;
glm::mat4 g_view_matrix, g_projection_matrix;

float g_previous_ticks = 0.0f,
g_accumulator = 0.0f;
bool started_game = false;


void initialise();
void process_input();
void update();
void render();
void shutdown();

//  GENERAL FUNCTIONS  //

constexpr int FONTBANK_SIZE = 16;
GLuint g_font_texture_id;

void draw_text(ShaderProgram* program, GLuint font_texture_id, std::string text,
    float font_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairsone for
    // each character. Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their
        //    position relative to the whole sentence)
        int spritesheet_index = (int)text[i];  // ascii value of character
        float offset = (font_size + spacing) * i;

        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float)(spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float)(spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (0.5f * font_size), -0.5f * font_size,
            offset + (0.5f * font_size), 0.5f * font_size,
            offset + (-0.5f * font_size), -0.5f * font_size,
            });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
            });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);

    program->set_model_matrix(model_matrix);
    glUseProgram(program->get_program_id());

    glVertexAttribPointer(program->get_position_attribute(), 2, GL_FLOAT, false, 0,
        vertices.data());
    glEnableVertexAttribArray(program->get_position_attribute());
    glVertexAttribPointer(program->get_tex_coordinate_attribute(), 2, GL_FLOAT, false, 0,
        texture_coordinates.data());
    glEnableVertexAttribArray(program->get_tex_coordinate_attribute());

    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

    glDisableVertexAttribArray(program->get_position_attribute());
    glDisableVertexAttribArray(program->get_tex_coordinate_attribute());
}

GLuint load_texture(const char* filepath)
{
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    GLuint texture_id;
    glGenTextures(NUMBER_OF_TEXTURES, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    stbi_image_free(image);

    return texture_id;
}

void initialise()
{
    //  GENERAL  //
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow(GAME_WINDOW_NAME,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    if (context == nullptr)
    {
        LOG("ERROR: Could not create OpenGL context.\n");
        shutdown();
    }

#ifdef _WINDOWS
    glewInit();
#endif

    //  VIDEO SETUP  //
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    //  MAP SET-UP  //
    GLuint map_texture_id = load_texture(MAP_TILESET_FILEPATH);
    g_game_state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, LEVEL_1_DATA, map_texture_id, 1.0f, 30, 32);

    //  GEORGE SET-UP  //

    GLuint player_texture_id = load_texture(SPRITESHEET_FILEPATH);
    GLuint enemy_texture_id = load_texture(ENEMY_FILEPATH);
    g_font_texture_id = load_texture(FONT_FILEPATH);


    int player_walking_animation[4][4] =
    {
        { 1, 5, 9, 13 },  // for George to move to the left,
        { 3, 7, 11, 15 }, // for George to move to the right,
        { 2, 6, 10, 14 }, // for George to move upwards,
        { 0, 4, 8, 12 }   // for George to move downwards
    };

    glm::vec3 acceleration = glm::vec3(0.0f, -4.905f, 0.0f);

    g_game_state.player = new Entity(
        player_texture_id,         // texture id
        5.0f,                      // speed
        acceleration,              // acceleration
        0.0f,                      // jumping power
        player_walking_animation,  // animation index sets
        0.0f,                      // animation time
        1,                         // animation frame amount
        0,                         // current animation index
        1,                         // animation column amount
        1,                         // animation row amount
        0.6f,                      // width
        0.9f,                       // height
        PLAYER
    );

    g_game_state.enemies = new Entity[3];

    /*for (int i = 0; i < 3; i++) {
        g_game_state.enemies[i].set_texture_id(enemy_texture_id);
        g_game_state.enemies[i].set_width(0.9f);
        g_game_state.enemies[i].set_height(0.9f);
        g_game_state.enemies[i].set_texture_id(enemy_texture_id);
    }*/
    for (int i = 0; i < 3; i++) {
        g_game_state.enemies[i] = Entity(
            enemy_texture_id,         // texture id
            5.0f,                      // speed
            acceleration,              // acceleration
            0.5f,                      // jumping power
            player_walking_animation,  // animation index sets
            0.0f,                      // animation time
            1,                         // animation frame amount
            0,                         // current animation index
            1,                         // animation column amount
            1,                         // animation row amount
            0.9f,                      // width
            0.9f,                       // height
            ENEMY
        );
    }

    g_game_state.enemies[0].set_position(glm::vec3(1.0f, 0.1f, 0.0f));
    g_game_state.enemies[0].set_ai_type(WALKER);
    g_game_state.enemies[1].set_position(glm::vec3(16.0f, -3.0f, 0.0f));
    g_game_state.enemies[1].set_ai_type(TOANDFROER);
    g_game_state.enemies[2].set_position(glm::vec3(37.0f, 2.1f, 0.0f));
    g_game_state.enemies[2].set_ai_type(JUMPER);

    for (int i = 0; i < 3; i++) {
        g_game_state.enemies[i].update(0.0f, NULL, NULL, 0, g_game_state.map);
    }


    // Jumping
    g_game_state.player->set_jumping_power(4.0f);

    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

    g_game_state.bgm = Mix_LoadMUS(BGM_FILEPATH);
    //    Mix_PlayMusic(g_game_state.bgm, -1);
    //    Mix_VolumeMusic(MIX_MAX_VOLUME / 16.0f);

    g_game_state.jump_sfx = Mix_LoadWAV(JUMP_SFX_FILEPATH);

    //  BLENDING  //
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input()
{
    g_game_state.player->set_movement(glm::vec3(0.0f));

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
        case SDL_QUIT:
        case SDL_WINDOWEVENT_CLOSE:
            g_app_status = TERMINATED;
            break;

        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                // Quit the game with a keystroke
                g_app_status = TERMINATED;
                break;

            case SDLK_SPACE:
                // Jump
                if (g_game_state.player->get_collided_bottom())
                {
                    g_game_state.player->jump();
                    Mix_PlayChannel(-1, g_game_state.jump_sfx, 0);
                }
                break;

            default:
                break;
            }

        default:
            break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT]) {
        g_game_state.player->move_left();

    }
    else if (key_state[SDL_SCANCODE_RIGHT]) g_game_state.player->move_right();

    if (glm::length(g_game_state.player->get_movement()) > 1.0f)
        g_game_state.player->normalise_movement();
}

void update()
{
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    if (started_game) {
        g_time_left -= delta_time;
    }
    if (g_time_left <= 0.0f && g_app_status == RUNNING){
        g_app_status = LOST;
    } 

    delta_time += g_accumulator;

    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }

    if (!started_game && g_game_state.player->get_position().x > 18.0f) {
        started_game = true;
    }

    while (delta_time >= FIXED_TIMESTEP)
    {
        g_game_state.player->update(FIXED_TIMESTEP, nullptr, g_game_state.enemies, 3,
            g_game_state.map);

        for (int i = 0; i < 3; i++) {
            g_game_state.enemies[i].update(FIXED_TIMESTEP, g_game_state.player,
                NULL, 0, g_game_state.map);
        }
        delta_time -= FIXED_TIMESTEP;
    }

    g_accumulator = delta_time;

    g_view_matrix = glm::mat4(1.0f);

    g_app_status = WON;
    g_enemies_left = 0;

    for (int i = 0; i < 3; i++) {
        if (g_game_state.enemies[i].get_is_active()) {
            g_app_status = RUNNING;
            g_enemies_left++;
        }
    }

    if (g_game_state.player->get_position().y < -6.0f && g_app_status == RUNNING){
        g_app_status = LOST;
    }

    // Camera Follows the player
    g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_game_state.player->get_position().x, 0.0f, 0.0f));
}

void render()
{
    g_shader_program.set_view_matrix(g_view_matrix);

    glClear(GL_COLOR_BUFFER_BIT);

    g_game_state.player->render(&g_shader_program);
    for (int i = 0; i < 3; i++) {
        g_game_state.enemies[i].render(&g_shader_program);
    }
    g_game_state.map->render(&g_shader_program);

    if (g_app_status == RUNNING) {
        if (!started_game) {
            text_to_display = "";
            draw_text(&g_shader_program, g_font_texture_id, "kill this guy", 0.5f, 0.05f,
                glm::vec3(0.0f, 2.5f, 0.0f));
            draw_text(&g_shader_program, g_font_texture_id, "time left:   ", 0.5f, 0.05f,
                glm::vec3(15.0f, 2.5f, 0.0f));
        }
        else {
            text_to_display = "time left: " + std::to_string((int)g_time_left);
        }
    }
    else if (g_app_status == WON) {
        text_to_display = "you win";
    }
    else if (g_app_status == LOST) {
        text_to_display = "next time lol";
    }

    draw_text(&g_shader_program, g_font_texture_id, text_to_display, 0.5f, 0.05f,
        glm::vec3(g_game_state.player->get_position().x - 3.5f, 2.5f, 0.0f));

    if (text_to_display != "" && g_app_status == RUNNING) {
        draw_text(&g_shader_program, g_font_texture_id, "enemies left: " + std::to_string((int)g_enemies_left), 0.3f, 0.02f,
            glm::vec3(g_game_state.player->get_position().x - 3.5f, 2.8f, 0.0f));
    }
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();

    delete[] g_game_state.enemies;
    delete    g_game_state.player;
    delete    g_game_state.map;
    Mix_FreeChunk(g_game_state.jump_sfx);
    Mix_FreeMusic(g_game_state.bgm);
}

//  GAME LOOP  //
int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status != TERMINATED)
    {
        process_input();
        update();
        render();
    }

    shutdown();
    return 0;
}
