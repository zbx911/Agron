#pragma once

const char* btcc(bool b, bool eot = true, bool pop = false)
{
    if (eot)
    {
        char c{};
        if (pop) c = 'd';
        if (b) return "Enable" + c;
        else return "Disable" + c;
    }
    else
    {
        if (b) return "True";
        else return "False";
    }
}

