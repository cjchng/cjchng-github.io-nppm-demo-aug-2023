//
// Created by cj on 2022-06-12.
//

#include <stdlib.h>
#include <iostream>

int main() {

// get and print shell environmental variable home
    std::cout << "CJ_NPPM_META = " << getenv("CJ_NPPM_META") << std::endl;
    std::cout << "MYENV = " << getenv("MYENV") << std::endl;

//set new shell environmental variable using putenv
    char mypath[]="TEMP=/my/new/temp/path/";
    putenv( mypath );

    std::cout << "TEMP = " << getenv("TEMP") << std::endl;

    return 0;
}