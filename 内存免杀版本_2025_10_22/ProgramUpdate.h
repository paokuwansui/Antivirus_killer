#ifndef MYDLL_H
#define MYDLL_H
#ifdef __cplusplus
#include <iostream>
extern "C" {
#endif
    __declspec(dllexport) int run(std::string filename);

#ifdef __cplusplus
}
#endif

#endif