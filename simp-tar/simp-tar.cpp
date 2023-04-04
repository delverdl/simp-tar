// tar-test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>

#include "CTarArchive.hpp"

int main()
{
    CTarArchive tar(R"(c:\json-out\t12x.tar)");

    tar.setMove(true);
    tar.addFileFromPath(R"(c:\json-out\hs_err_pid25596.log)");
    tar.addFileFromPath(R"(c:\json-out\hs_err_pid31472.log)");
    tar.addFileFromPath(R"(c:\json-out\hs_err_pid40616.log)");
    tar.addFileFromPath(R"(c:\json-out\hs_err_pid52652.log)");
    
    tar.close();
}
