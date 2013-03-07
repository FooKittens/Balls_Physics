#ifndef GAMETIMER_H
#define GAMETIMER_H
#include <windows.h>

/* Class used to measure time with greater precision through
 * Win32's QueryPerformanceCounter api. This requires the prescence
 * of a high performance counter on the CPU, which should be present on most
 * hardware more recent than 10 years. */

class GameTimer
{
public:
  // Constructor
  GameTimer();

  // Destructor
  ~GameTimer();

  // Starts the timer.
  bool Start();

  // Sets the running flag to false.
  void Stop();

  // Returns the time since the timer was started.
  double TimeSinceStart();

  /* Returns the elapsed time since 
   * the last call to DeltaTime() */
  double DeltaTime();
  
  // Check if the timer is running
  inline int IsRunning() { return running; }



private:
  // Returns the elapsed milliseconds between two LARGE_INTEGERs
  double DeltaSeconds(const LARGE_INTEGER &later,
                          const LARGE_INTEGER &earlier);

  // True if the timer is running.
  bool running;

  LARGE_INTEGER startTick;
  LARGE_INTEGER lastTick;
  LARGE_INTEGER ticksPerSec;
  LARGE_INTEGER lastDeltaTick;
  LARGE_INTEGER thisDeltaTick;
};

#endif