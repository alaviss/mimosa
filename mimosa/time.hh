#ifndef MIMOSA_TIME_HH
# define MIMOSA_TIME_HH

# include <cstdint>
# include <ctime>
# include <stdexcept>

# include <unistd.h>

namespace mimosa
{
  // signed to ease time substraction
  typedef ::int64_t Time;

  const Time nanosecond  = 1;
  const Time microsecond = 1000 * nanosecond;
  const Time millisecond = 1000 * microsecond;
  const Time second      = 1000 * millisecond;
  const Time minute      = 60 * second;
  const Time hour        = 60 * minute;
  const Time day         = 24 * hour;

  inline Time realTime()
  {
    ::timespec tp;
    int ret = ::clock_gettime(CLOCK_REALTIME, &tp);
    if (ret)
      throw std::runtime_error("clock_gettime");
    return tp.tv_nsec * nanosecond + tp.tv_sec * second;
  }

  inline Time monotonicTime()
  {
    ::timespec tp;
    int ret = ::clock_gettime(CLOCK_MONOTONIC, &tp);
    if (ret)
      throw std::runtime_error("clock_gettime");
    return tp.tv_nsec * nanosecond + tp.tv_sec * second;
  }

  inline Time realTimeCoarse()
  {
    ::timespec tp;
    int ret = ::clock_gettime(CLOCK_REALTIME_COARSE, &tp);
    if (ret)
      throw std::runtime_error("clock_gettime");
    return tp.tv_nsec * nanosecond + tp.tv_sec * second;
  }

  inline Time monotonicTimeCoarse()
  {
    ::timespec tp;
    int ret = ::clock_gettime(CLOCK_MONOTONIC_COARSE, &tp);
    if (ret)
      throw std::runtime_error("clock_gettime");
    return tp.tv_nsec * nanosecond + tp.tv_sec * second;
  }

  inline Time time()
  {
    return monotonicTimeCoarse();
  }

  inline void toTimeSpec(Time time, ::timespec * tp)
  {
    tp->tv_sec = time / second;
    tp->tv_nsec = time % second;
  }

  inline void sleep(Time duration) { ::usleep(duration / microsecond); }
}

#endif /* !MIMOSA_TIME_HH */