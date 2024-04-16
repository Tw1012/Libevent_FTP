#pragma once

#include <iostream>
using namespace std;

#define TEST //进行测试

#ifdef TEST
#define testout(msg) cout << msg << endl << flush
#else 
#define testout(msg) 
#endif
