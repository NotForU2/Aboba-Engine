#include <SDL2/SDL.h>
#include "core/Engine.hpp"
#include <iostream>

int main(int, char **)
{
  Engine app;
  if (app.Initialize())
  {
    app.Run();
  }

  return 0;
}