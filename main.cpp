#include <Windows.h>
#include <gdiplus.h>
#include "Window.h"

#pragma comment(lib, "Gdiplus.lib")

/// ------- SETTINGS --------- ///
const int ScreenWidth = 800;
const int ScreenHeight = 640;

int WINAPI WinMain(HINSTANCE inst, HINSTANCE pinst, TCHAR *cmd, int cmdshow)
{
  Window window(inst, ScreenWidth, ScreenHeight);
  
  if(!window.Initialize() || !window.Run())
  {
    MessageBox(NULL, TEXT("Application was terminated unexpectadly!"),
      TEXT("Error"), MB_OK | MB_ICONERROR);
  }

  return 0;
}
