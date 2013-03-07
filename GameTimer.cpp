#include "GameTimer.h"


GameTimer::GameTimer()
{
  running = false;
  QueryPerformanceFrequency(&ticksPerSec);
}

GameTimer::~GameTimer()
{

}


bool GameTimer::Start()
{
  if(!QueryPerformanceCounter(&startTick))
  {
    return false;
  }
  
  running = true;

  lastTick = startTick;
  lastDeltaTick = startTick;
  
  return true;
}

void GameTimer::Stop()
{
  running = false;
}

double GameTimer::TimeSinceStart()
{
  // Return 0 if the timer isn't running.
  if(!running)
  {
    return 0.0;
  }

  QueryPerformanceCounter(&lastTick);

  return DeltaSeconds(lastTick, startTick);
}

double GameTimer::DeltaTime()
{
  if(!running)
  {
    return 0.0;
  }

  QueryPerformanceCounter(&thisDeltaTick);

  double ms = DeltaSeconds(thisDeltaTick, lastDeltaTick);

  lastDeltaTick = thisDeltaTick;

  return ms;
}

double GameTimer::DeltaSeconds(const LARGE_INTEGER &later,
                                  const LARGE_INTEGER &earlier)
{
  __int64 elapsedTicks = later.QuadPart - earlier.QuadPart;
  double elapsedSeconds = elapsedTicks / (double)ticksPerSec.QuadPart;

  // Convert to millisceonds
  return elapsedSeconds;
}
