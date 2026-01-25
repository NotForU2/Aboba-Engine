#pragma once

struct Position
{
  float x;
  float y;
};

struct Velocity
{
  float vx;
  float vy;
};

struct RenderData
{
  int size;
  unsigned char r, g, b;
};

struct Destination
{
  float targetX;
  float targetY;
};