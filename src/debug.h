//
// Created by Lin on 2024/11/18.
//

#ifndef L_DEBUG_H
#define L_DEBUG_H

#if PROFILE
#include <chrono>
#include <iostream>
class DEBUG_NODE
{
#if __APPLE__
    std::chrono::steady_clock::time_point time_begin;
    std::chrono::steady_clock::time_point time_end;
#endif

#if __linux__
    std::chrono::system_clock::time_point time_begin;
    std::chrono::system_clock::time_point time_end;
#endif
    long long elapsed_time;
    const char* prefix;
public:
    DEBUG_NODE()
    {
        time_begin = std::chrono::high_resolution_clock::now();
    }
    DEBUG_NODE(const char* pre)
    {
        prefix = pre;
        time_begin = std::chrono::high_resolution_clock::now();
    }
    ~DEBUG_NODE()
    {
        time_end = std::chrono::high_resolution_clock::now();
        elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(time_end - time_begin).count();
        printf("%s:%.2f fps, %0.2f s\n", prefix, 1000.f / elapsed_time, elapsed_time * 1.0f / 1000.f);
    }
};
#define PERFORMANCE_DEBUG(prefix) auto ___debug = DEBUG_NODE(#prefix);
#else
#define PERFORMANCE_DEBUG(prefix)
#endif

#if DEBUG
#define RUNTIME_ASSERT(cond, msg,...) \
    do { \
        if (!(cond)) { \
            std::cerr << "Assertion failed: " << msg << "\n"; \
            std::cerr << "File: " << __FILE__ << ", Line: " << __LINE__ << "\n"; \
            std::exit(EXIT_FAILURE); \
        } \
    } while (0)
#define DEBUG_VAR int ___debug = 0;
#else
#define RUNTIME_ASSERT(cond, msg)
#define DEBUG_VAR
#endif
#endif //DEBUG_H


