//
// Created by Lin on 2024/11/18.
//

#ifndef L_DEBUG_H
#define L_DEBUG_H
#include <chrono>
#include <cstdlib>
#include <iostream>


#define TIME_RUN_BEGIN()\
auto frame_start = std::chrono::high_resolution_clock::now();
#define TIME_RUN_END()\
auto frame_end = std::chrono::high_resolution_clock::now();\
auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(frame_end - frame_start).count();
#define PRINT_RUN_TIME(PRE)\
printf("%s:%.2f fps, %0.2f s\n", #PRE,1000.f/elapsed_time,elapsed_time*1.0f/1000.f);


#define RUNTIME_ASSERT(cond, msg) \
    do { \
        if (!(cond)) { \
            std::cerr << "Assertion failed: " << msg << "\n"; \
            std::cerr << "File: " << __FILE__ << ", Line: " << __LINE__ << "\n"; \
            std::exit(EXIT_FAILURE); \
        } \
    } while (0)

#endif //DEBUG_H
