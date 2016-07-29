#pragma once

#if defined(__linux) || defined(__linux__)
# define MIMOSA_LINUX
# define MIMOSA_UNIX
#elif defined(__APPLE__)
# define MIMOSA_MAC
# define MIMOSA_UNIX
#elif defined(__unix__)
# define MIMOSA_UNIX
#elif defined(__WIN32__)
# define MIMOSA_WIN
#endif
