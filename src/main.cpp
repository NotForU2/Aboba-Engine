#include "core/Engine.hpp"

int main(int, char **)
{
  Engine app;
  if (app.Init())
  {
    app.Run();
  }

  return 0;
}