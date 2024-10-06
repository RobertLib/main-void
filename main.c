#include "raylib.h"
#include <stdio.h>
#include <math.h>

#define MAP_WIDTH 25
#define MAP_HEIGHT 14
#define TILE_SIZE 16
#define GRAVITY 30
#define SCALE 2

typedef struct Tile
{
  char symbol;
  Rectangle textureRect;
} Tile;

#define TILE_COUNT 3

Tile tiles[TILE_COUNT];

#define LEVEL_COUNT 2

int level = 0;

char tilemap[LEVEL_COUNT][MAP_HEIGHT * MAP_WIDTH + 1] = {
    "GGGGGGGGGGGGGGGGGGGGGGGGG"
    "G                       G"
    "G                       G"
    "G                       G"
    "G       E               G"
    "G       E               G"
    "G  GGLGGGG              G"
    "G    L                  G"
    "G    L                  G"
    "G  GGGGGGGGLGG          G"
    "G          L            G"
    "G          L      G     G"
    "G          L     GGG    G"
    "GGGGGGGGGGGGGGGGGGGGGGGGG",
    "GGGGGGGGGGGGGGGGGGGGGGGGG"
    "G                       G"
    "G                       G"
    "G                       G"
    "G                       G"
    "G                       G"
    "G                       G"
    "G                       G"
    "G                       G"
    "G                       G"
    "G                       G"
    "G                  E    G"
    "G                  E    G"
    "GGGGGGGGGGGGGGGGGGGGGGGGG"};

#define GET_TILE(x, y) (tilemap[level][(y) * MAP_WIDTH + (x)])

void initTiles()
{
  tiles[0].symbol = 'G';
  tiles[0].textureRect = (Rectangle){0, 0, TILE_SIZE, TILE_SIZE};
  tiles[1].symbol = 'L';
  tiles[1].textureRect = (Rectangle){TILE_SIZE, 0, TILE_SIZE, TILE_SIZE};
  tiles[2].symbol = 'E';
  tiles[2].textureRect = (Rectangle){TILE_SIZE * 2, 0, TILE_SIZE, TILE_SIZE};
}

void drawTiles(Texture2D *tileset)
{
  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      char symbol = GET_TILE(x, y);

      if (symbol == ' ')
        continue;

      for (int i = 0; i < TILE_COUNT; i++)
      {
        if (tiles[i].symbol == symbol)
        {
          Rectangle destRect = {
              x * TILE_SIZE * SCALE,
              y * TILE_SIZE * SCALE,
              tiles[i].textureRect.width * SCALE,
              tiles[i].textureRect.height * SCALE};

          DrawTexturePro(
              *tileset,
              tiles[i].textureRect,
              destRect,
              (Vector2){0, 0},
              0,
              WHITE);
          break;
        }
      }
    }
  }
}

bool checkCollisionWithMap(Rectangle rect)
{
  int centerX = rect.x + rect.width / 2;
  int centerY = rect.y + rect.height / 2;

  int startX = (centerX - rect.width / 2) / TILE_SIZE;
  int endX = (centerX + rect.width / 2) / TILE_SIZE;
  int startY = (centerY - rect.height / 2) / TILE_SIZE;
  int endY = (centerY + rect.height / 2) / TILE_SIZE;

  if (startX < 0)
    startX = 0;
  if (endX >= MAP_WIDTH)
    endX = MAP_WIDTH - 1;
  if (startY < 0)
    startY = 0;
  if (endY >= MAP_HEIGHT)
    endY = MAP_HEIGHT - 1;

  for (int y = startY; y <= endY; y++)
  {
    for (int x = startX; x <= endX; x++)
    {
      char symbol = GET_TILE(x, y);

      if (symbol == ' ')
        continue;

      if (symbol == 'G')
      {
        Rectangle tileRect = {x * TILE_SIZE, y * TILE_SIZE, TILE_SIZE, TILE_SIZE};

        if (CheckCollisionRecs(rect, tileRect))
        {
          return true;
        }
      }
    }
  }

  return false;
}

void saveTilemapToFile()
{
  FILE *file = fopen("tilemap.txt", "w");

  if (file == NULL)
  {
    perror("Unable to open file");
    return;
  }

  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      fputc(tilemap[level][y * MAP_WIDTH + x], file);
    }

    fputc('\n', file);
  }

  fclose(file);
}

char currentTile = ' ';

void handleMouseClick(int mouseX, int mouseY)
{
  int tileX = mouseX / (TILE_SIZE * SCALE);
  int tileY = mouseY / (TILE_SIZE * SCALE);

  if (tileX >= 0 && tileX < MAP_WIDTH && tileY >= 0 && tileY < MAP_HEIGHT)
  {
    tilemap[level][tileY * MAP_WIDTH + tileX] = currentTile;
  }
}

typedef struct Player
{
  Vector2 pos;
  Vector2 vel;
  Rectangle textureRect;
  float speed;
  bool isOnGround;
} Player;

Player player = {0};

void initPlayer()
{
  player.pos = (Vector2){TILE_SIZE * 2, TILE_SIZE * 2};
  player.vel = (Vector2){0, 0};
  player.textureRect = (Rectangle){0, 0, TILE_SIZE, TILE_SIZE * 2};
  player.speed = 20;
  player.isOnGround = false;
}

void updatePlayer(float dt)
{
  int centerX = player.pos.x + player.textureRect.width / 2;
  int topY = player.pos.y + 1;
  int centerY = topY + player.textureRect.height / 2;
  int bottomY = topY + player.textureRect.height;

  bool onLadder =
      GET_TILE(centerX / TILE_SIZE, topY / TILE_SIZE) == 'L' ||
      GET_TILE(centerX / TILE_SIZE, centerY / TILE_SIZE) == 'L' ||
      GET_TILE(centerX / TILE_SIZE, bottomY / TILE_SIZE) == 'L';

  bool onEnd = GET_TILE(centerX / TILE_SIZE, centerY / TILE_SIZE) == 'E';

  if (onEnd)
  {
    level = (level + 1) % LEVEL_COUNT;
    initPlayer();
  }

  if (IsKeyDown(KEY_RIGHT))
    player.vel.x += player.speed * dt;
  if (IsKeyDown(KEY_LEFT))
    player.vel.x -= player.speed * dt;

  if (onLadder &&
      centerX % TILE_SIZE > TILE_SIZE / 4 &&
      centerX % TILE_SIZE < TILE_SIZE - TILE_SIZE / 4)
  {
    player.vel.y = 0;

    if (IsKeyDown(KEY_UP))
    {
      player.vel.x = 0;
      player.vel.y -= player.speed * 2 * dt;
      player.pos.x = (int)(centerX / TILE_SIZE) * TILE_SIZE;
    }
    if (IsKeyDown(KEY_DOWN))
    {
      player.vel.x = 0;
      player.vel.y += player.speed * 2 * dt;
      player.pos.x = (int)(centerX / TILE_SIZE) * TILE_SIZE;
    }
  }
  else
  {
    if (IsKeyPressed(KEY_UP) && player.isOnGround)
      player.vel.y -= 10;

    player.vel.y += GRAVITY * dt;
  }

  Vector2 newPos = {player.pos.x + player.vel.x, player.pos.y + player.vel.y};

  for (int i = 0; i < 3; i++)
  {
    if (checkCollisionWithMap((Rectangle){
            newPos.x,
            player.pos.y,
            player.textureRect.width,
            player.textureRect.height}))
    {
      player.vel.x /= 2;
      newPos.x = player.pos.x + player.vel.x;
    }
    else
    {
      player.pos.x = newPos.x;
      break;
    }
  }

  for (int i = 0; i < 3; i++)
  {
    if (checkCollisionWithMap((Rectangle){
            player.pos.x,
            newPos.y,
            player.textureRect.width,
            player.textureRect.height}))
    {
      player.pos.y = ceil(player.pos.y);
      player.vel.y /= 2;
      newPos.y = player.pos.y + player.vel.y;
      player.isOnGround = true;
    }
    else
    {
      player.pos.y = newPos.y;
      player.isOnGround = false;
      break;
    }
  }

  player.vel.x *= 0.8;
  player.vel.y *= 0.8;
}

void drawPlayer(Texture2D *spritesheet)
{
  Rectangle destRect = {
      player.pos.x * SCALE,
      player.pos.y * SCALE,
      player.textureRect.width * SCALE,
      player.textureRect.height * SCALE};

  DrawTexturePro(
      *spritesheet,
      player.textureRect,
      destRect,
      (Vector2){0, 0},
      0,
      WHITE);
}

char keys[] = {KEY_SPACE, KEY_G, KEY_L, KEY_E};
int numKeys = sizeof(keys) / sizeof(keys[0]);

int main(void)
{
  int screenWidth = 800;
  int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "Main Void");

  SetTargetFPS(60);

  Texture2D tileset = LoadTexture("tileset.png");
  Texture2D spritesheet = LoadTexture("spritesheet.png");

  initTiles();
  initPlayer();

  while (!WindowShouldClose())
  {
    if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_ENTER))
      ToggleFullscreen();

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S))
      saveTilemapToFile();

    for (int i = 0; i < numKeys; i++)
    {
      if (IsKeyPressed(keys[i]))
      {
        currentTile = keys[i];
        break;
      }
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
      Vector2 mousePosition = GetMousePosition();
      handleMouseClick((int)mousePosition.x, (int)mousePosition.y);
    }

    float dt = GetFrameTime();

    updatePlayer(dt);

    BeginDrawing();

    ClearBackground(BLACK);

    drawTiles(&tileset);
    drawPlayer(&spritesheet);

    EndDrawing();
  }

  UnloadTexture(tileset);
  UnloadTexture(spritesheet);

  CloseWindow();

  return 0;
}
