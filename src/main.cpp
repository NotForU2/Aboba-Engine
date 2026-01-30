#include <SDL2/SDL.h>
#include "core/Engine.hpp"
#include <iostream>

int main(int, char **)
{
  Engine app;
  if (app.Init())
  {
    app.Run();
  }

  return 0;
}