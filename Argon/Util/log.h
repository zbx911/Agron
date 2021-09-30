#pragma once

#include <iostream>

template <typename T>
void Log(T msg, bool nl = true)
{
    std::cout << msg;
    if (nl) std::cout << "\n";
}