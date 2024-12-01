#include <pthread.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <queue>
#include <mutex>
#include <cctype>

using namespace std;

// Global variables
vector<string> inFiles;
vector<pair<string, int>> partialResult;
vector<vector<pair<string, int>>> finalListForReduce;
pthread_barrier_t barrier;
mutex list_file_mutex, add_sol_mutex;
int numMap;
int numReduce;

void threadFunction(void *arg) {

}

int main(int argc, char **argv) {
    // Check if the number of arguments is correct
    if (argc < 4) {
        cout << "Usage: ./tema1 <num_threads> <num_map> <num_reduce> <in_file>\n";
        return -1;
    }
    int numMappers = std::stoi(argv[1]);
    int numReducers = std::stoi(argv[2]);
    string inFileName = argv[3];
    ifstream fin(inFileName);
    int numFiles = 0;
    fin >> numFiles;
    for (int i = 0; i < numFiles; i++) {
        string fileName;
        fin >> fileName;
        inFiles.push_back(fileName);
    }
    return 0;
}