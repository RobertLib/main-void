#include "raylib.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

#define MAP_WIDTH 33
#define MAP_HEIGHT 18
#define TILE_SIZE 12
#define GRAVITY 9.81

typedef struct Tile
{
  char symbol;
  Rectangle textureRect;
} Tile;

#define TILE_COUNT 6

Tile tiles[TILE_COUNT];

#define LEVEL_COUNT 2

int level = 0;
float time = 200;

char tilemap[LEVEL_COUNT][MAP_HEIGHT * MAP_WIDTH + 1] = {
    "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG"
    "G                               G"
    "G                               G"
    "G                               G"
    "G                               G"
    "G                               G"
    "G                               G"
    "G                               G"
    "G       D                       G"
    "G  K                            G"
    "G  GGLGGGG                      G"
    "G    L                          G"
    "G    L       K                  G"
    "G  GGGGGGGGLGG                  G"
    "G          L                    G"
    "G  P       L               G    G"
    "G          L      SS      GGG   G"
    "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG",
    "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG"
    "G                               G"
    "G                               G"
    "G                               G"
    "G                               G"
    "G                               G"
    "G                               G"
    "G                               G"
    "G                               G"
    "G                               G"
    "G                               G"
    "G                               G"
    "G                               G"
    "G                               G"
    "G                               G"
    "G  P                        D   G"
    "G            K                  G"
    "GGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGGG"};

char initialTilemap[LEVEL_COUNT][MAP_HEIGHT * MAP_WIDTH + 1];

void initTilemap()
{
  for (int i = 0; i < LEVEL_COUNT; i++)
  {
    strncpy(initialTilemap[i], tilemap[i], MAP_HEIGHT * MAP_WIDTH);
    initialTilemap[i][MAP_HEIGHT * MAP_WIDTH] = '\0';
  }
}

void resetTilemap()
{
  for (int i = 0; i < LEVEL_COUNT; i++)
  {
    strncpy(tilemap[i], initialTilemap[i], MAP_HEIGHT * MAP_WIDTH);
    tilemap[i][MAP_HEIGHT * MAP_WIDTH] = '\0';
  }
}

#define GET_TILE(x, y) (tilemap[level][(y) * MAP_WIDTH + (x)])

void initTiles()
{
  tiles[0].symbol = 'G';
  tiles[0].textureRect = (Rectangle){0, 0, TILE_SIZE, TILE_SIZE};
  tiles[1].symbol = 'L';
  tiles[1].textureRect = (Rectangle){TILE_SIZE, 0, TILE_SIZE, TILE_SIZE};
  tiles[2].symbol = 'D';
  tiles[2].textureRect = (Rectangle){TILE_SIZE * 2, 0, TILE_SIZE, TILE_SIZE * 2};
  tiles[3].symbol = 'O';
  tiles[3].textureRect = (Rectangle){TILE_SIZE * 3, 0, TILE_SIZE, TILE_SIZE * 2};
  tiles[4].symbol = 'K';
  tiles[4].textureRect = (Rectangle){TILE_SIZE * 4, 0, TILE_SIZE, TILE_SIZE};
  tiles[5].symbol = 'S';
  tiles[5].textureRect = (Rectangle){TILE_SIZE * 5, 0, TILE_SIZE, TILE_SIZE};
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
              x * TILE_SIZE,
              y * TILE_SIZE,
              tiles[i].textureRect.width,
              tiles[i].textureRect.height};

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

Vector2 checkCollisionWithMap(Rectangle rect)
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
          return (Vector2){x * TILE_SIZE, y * TILE_SIZE};
        }
      }
    }
  }

  return (Vector2){-1, -1};
}

void openDoor()
{
  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      if (GET_TILE(x, y) == 'D')
        GET_TILE(x, y) = 'O';
    }
  }
}

int countKeysInLevel()
{
  int count = 0;

  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      if (GET_TILE(x, y) == 'K')
        count++;
    }
  }

  return count;
}

void pickUpKey(int x, int y)
{
  GET_TILE(x / TILE_SIZE, y / TILE_SIZE) = ' ';

  if (countKeysInLevel() == 0)
    openDoor();
}

typedef struct Player
{
  Vector2 pos;
  Vector2 vel;
  Rectangle textureRect;
  float speed;
  float jumpHeight;
  bool isOnGround;
} Player;

Player player = {0};

Vector2 findPlayerPosition()
{
  for (int y = 0; y < MAP_HEIGHT; y++)
  {
    for (int x = 0; x < MAP_WIDTH; x++)
    {
      if (GET_TILE(x, y) == 'P')
      {
        return (Vector2){x * TILE_SIZE, y * TILE_SIZE};
      }
    }
  }

  return (Vector2){-1, -1};
}

void initPlayer()
{
  player.pos = findPlayerPosition();
  player.vel = (Vector2){0, 0};
  player.textureRect = (Rectangle){0, 0, TILE_SIZE, TILE_SIZE * 2};
  player.speed = 12;
  player.jumpHeight = 4;
  player.isOnGround = false;
}

void changeLevel()
{
  level = (level + 1) % LEVEL_COUNT;
  time = 200;

  if (level == 0)
    resetTilemap();

  initPlayer();
}

void updatePlayer(float dt)
{
  if (IsKeyDown(KEY_RIGHT))
    player.vel.x += player.speed * dt;
  if (IsKeyDown(KEY_LEFT))
    player.vel.x -= player.speed * dt;

  float centerX = player.pos.x + player.textureRect.width / 2;
  float topY = player.pos.y;
  float topHalfY = player.pos.y + player.textureRect.height / 4;
  float centerY = topY + player.textureRect.height / 2;
  float bottomHalfY = player.pos.y + player.textureRect.height * 3 / 4;
  float bottomY = topY + player.textureRect.height;

  bool topYOnLadder = GET_TILE((int)centerX / TILE_SIZE, (int)topY / TILE_SIZE) == 'L';
  bool centerYOnLadder = GET_TILE((int)centerX / TILE_SIZE, (int)centerY / TILE_SIZE) == 'L';
  bool bottomYOnLadder = GET_TILE((int)centerX / TILE_SIZE, (int)bottomY / TILE_SIZE) == 'L';
  bool onLadder = topYOnLadder || centerYOnLadder || bottomYOnLadder;
  bool aboveLadder =
      !topYOnLadder && !centerYOnLadder && bottomYOnLadder &&
      (int)player.pos.y % TILE_SIZE == 0;

  if (onLadder &&
      (int)centerX % TILE_SIZE > TILE_SIZE / 4 &&
      (int)centerX % TILE_SIZE < TILE_SIZE - TILE_SIZE / 4)
  {
    if (player.vel.x == 0)
      player.vel.y = 0;

    if (IsKeyDown(KEY_UP))
    {
      player.vel = (Vector2){0, 0};
      player.vel.y -= player.speed * 2 * dt;

      if (centerYOnLadder || topYOnLadder)
        player.pos.x = (int)centerX / TILE_SIZE * TILE_SIZE;

      if (aboveLadder)
      {
        player.pos.y = (int)topY / TILE_SIZE * TILE_SIZE;
        player.vel.y = 0;
      }
    }
    if (IsKeyDown(KEY_DOWN))
    {
      player.vel = (Vector2){0, 0};
      player.vel.y += player.speed * 2 * dt;

      if (centerYOnLadder || bottomYOnLadder)
        player.pos.x = (int)centerX / TILE_SIZE * TILE_SIZE;
    }
  }
  else
    player.vel.y += GRAVITY * dt;

  if (IsKeyPressed(KEY_UP) && (player.isOnGround || aboveLadder))
    player.vel.y = -player.jumpHeight;

  bool onKey = GET_TILE((int)centerX / TILE_SIZE, (int)topHalfY / TILE_SIZE) == 'K' ||
               GET_TILE((int)centerX / TILE_SIZE, (int)bottomHalfY / TILE_SIZE) == 'K';

  if (onKey)
  {
    pickUpKey(centerX, topHalfY);
    pickUpKey(centerX, bottomHalfY);
  }

  bool onOpenDoor =
      GET_TILE((int)centerX / TILE_SIZE, (int)topHalfY / TILE_SIZE) == 'O' ||
      GET_TILE((int)centerX / TILE_SIZE, (int)bottomHalfY / TILE_SIZE) == 'O';

  if (onOpenDoor)
    changeLevel();

  bool onSpike = GET_TILE((int)centerX / TILE_SIZE, (int)bottomHalfY / TILE_SIZE) == 'S';

  if (onSpike)
    initPlayer();

  Vector2 newPos = {player.pos.x + player.vel.x, player.pos.y + player.vel.y};

  bool skippedVerticalCollision = false;

  for (int i = 0; i < 4; i++)
  {
    Vector2 collisionPos = checkCollisionWithMap((Rectangle){
        player.pos.x,
        newPos.y,
        player.textureRect.width,
        player.textureRect.height});

    if (collisionPos.y != -1 && bottomY <= collisionPos.y)
    {
      player.pos.y = ceil(player.pos.y);
      player.vel.y /= 2;
      newPos.y = player.pos.y + player.vel.y;
      player.isOnGround = true;
    }
    else
    {
      if (collisionPos.y > -1)
        skippedVerticalCollision = true;

      player.pos.y = newPos.y;
      player.isOnGround = false;
      break;
    }
  }

  for (int i = 0; i < 4; i++)
  {
    Vector2 collisionPos = checkCollisionWithMap((Rectangle){
        newPos.x,
        player.pos.y,
        player.textureRect.width,
        player.textureRect.height});

    if (collisionPos.x != -1 && !skippedVerticalCollision)
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

  player.vel.x *= 0.8;
  player.vel.y *= 0.9;
}

void drawPlayer(Texture2D *spritesheet)
{
  Rectangle destRect = {
      player.pos.x,
      player.pos.y,
      player.textureRect.width,
      player.textureRect.height};

  DrawTexturePro(
      *spritesheet,
      player.textureRect,
      destRect,
      (Vector2){0, 0},
      0,
      WHITE);
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
  int tileX = mouseX / (GetScreenWidth() / MAP_WIDTH);
  int tileY = mouseY / (GetScreenHeight() / MAP_HEIGHT);

  if (tileX >= 0 && tileX < MAP_WIDTH && tileY >= 0 && tileY < MAP_HEIGHT)
  {
    tilemap[level][tileY * MAP_WIDTH + tileX] = currentTile;
  }
}

char keys[] = {KEY_SPACE, KEY_G, KEY_L, KEY_D, KEY_K, KEY_S};
int numKeys = sizeof(keys) / sizeof(keys[0]);

int main(void)
{
  int screenWidth = 800;
  int screenHeight = 450;

  InitWindow(screenWidth, screenHeight, "Main Void");

  SetTargetFPS(60);

  Texture2D tileset = LoadTexture("tileset.png");
  Texture2D spritesheet = LoadTexture("spritesheet.png");

  RenderTexture2D renderTexture = LoadRenderTexture(
      MAP_WIDTH * TILE_SIZE,
      MAP_HEIGHT * TILE_SIZE);

  initTilemap();
  initTiles();
  initPlayer();

  while (!WindowShouldClose())
  {
    if (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_ENTER))
    {
      ToggleFullscreen();
    }

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

    Vector2 origin = {
        (GetScreenWidth() - GetRenderWidth() / GetWindowScaleDPI().x) / 2,
        (GetScreenHeight() - GetRenderHeight() / GetWindowScaleDPI().y) / 2};

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
    {
      Vector2 mousePosition = GetMousePosition();

      handleMouseClick(
          (int)(mousePosition.x + origin.x),
          (int)(mousePosition.y + origin.y));
    }

    float dt = GetFrameTime();

    time -= dt;

    updatePlayer(dt);

    BeginTextureMode(renderTexture);

    ClearBackground(BLACK);

    drawTiles(&tileset);
    drawPlayer(&spritesheet);

    DrawText(TextFormat("Level: %d", level + 1), 20, 18, 9, WHITE);
    DrawText(TextFormat("Time: %.0f", time), 335, 18, 9, WHITE);

    EndTextureMode();

    BeginDrawing();

    DrawTexturePro(
        renderTexture.texture,
        (Rectangle){0, 0, renderTexture.texture.width, -renderTexture.texture.height},
        (Rectangle){0, 0, GetScreenWidth(), GetScreenHeight()},
        origin,
        0,
        WHITE);

    EndDrawing();
  }

  UnloadTexture(tileset);
  UnloadTexture(spritesheet);

  UnloadRenderTexture(renderTexture);

  CloseWindow();

  return 0;
}
