#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <string.h>
#include <stdlib.h>

using namespace std;

int max_lines;

int run_cmd (const char* cmd, int show_result=0);
string read_file(const char* file);
int process_command (string value);
