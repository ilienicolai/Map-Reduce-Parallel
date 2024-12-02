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
queue<pair<string, int>> inFiles;
vector<pair<string, int>> partialResult;
pthread_barrier_t barrier;
pthread_mutex_t list_file_mutex, add_sol_mutex;
queue<char> alphabet;
int numMap;
int numReduce;

string normalizeWord(const string& word) {
    string normalized;
    for (char c : word) {
        if (std::isalpha(c)) {
            normalized += std::tolower(c);
        }
    }
    return normalized;
}

void *threadFunction(void *arg) {
    int thread_id = *((int *) arg);
    if (thread_id < numMap) {
        while (true) {
            string fileName;
            int index;
            pthread_mutex_lock(&list_file_mutex);
            if (inFiles.empty()) {
                pthread_mutex_unlock(&list_file_mutex);
                break;
            }
            fileName = inFiles.front().first;
            index = inFiles.front().second;
            inFiles.pop();
            pthread_mutex_unlock(&list_file_mutex);
            ifstream fin(fileName);
            string word;
            vector<pair<string, int>> localResult;
            while (fin >> word) {
                string normalized;
                for (char c : word) {
                    if (std::isalpha(c)) {
                        normalized += string(1,tolower(c));
                    }
                }
                if (normalized.size() > 0) {
                    localResult.push_back(make_pair(normalized, index));
                }
            }
            pthread_mutex_lock(&add_sol_mutex);
            partialResult.insert(partialResult.end(), localResult.begin(), localResult.end());
            pthread_mutex_unlock(&add_sol_mutex);
        }
    }
    pthread_barrier_wait(&barrier);
    if (thread_id >= numMap) {
        
        while (true) {
            map<string , set<int>> localResult;
            char letter;
            pthread_mutex_lock(&list_file_mutex);
            if (alphabet.empty()) {
                pthread_mutex_unlock(&list_file_mutex);
                break;
            }
            letter = alphabet.front();
            alphabet.pop();
            pthread_mutex_unlock(&list_file_mutex);
            for (auto &it : partialResult) {
                if (it.first[0] == letter) {
                    localResult[it.first].insert(it.second);
                }
            }
            
            vector<pair<string, set<int>>> sortedResult;
            map<string, set<int>>::iterator it;
            for (it = localResult.begin(); it != localResult.end(); it++) {
                sortedResult.push_back(make_pair(it->first, it->second));
            }
            sort(sortedResult.begin(), sortedResult.end(), [](const pair<string, set<int>> &a, const pair<string, set<int>> &b) {
                if (a.second.size() == b.second.size()) {
                    return a.first < b.first;
                }
                return a.second.size() > b.second.size();
            });
            ofstream fout(string(1, letter) + ".txt");
            vector<pair<string, set<int>>>::iterator it2;
            set<int>::iterator it3;
            for(it2 = sortedResult.begin(); it2 != sortedResult.end(); it2++) {
                fout << it2->first << ": [";
                for (it3 = it2->second.begin(); it3 != it2->second.end(); it3++) {
                    fout << *it3;
                    if (next(it3) != it2->second.end()) {
                        fout << " ";
                    }
                }
                fout << "]" << endl;
            }
            fout.close();
        }
    }
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    // Check if the number of arguments is correct
    if (argc < 4) {
        cout << "Usage: ./tema1 <num_map> <num_reduce> <in_file>\n";
        return -1;
    }
    int numMappers = std::stoi(argv[1]);
    int numReducers = std::stoi(argv[2]);
    string inFileName = argv[3];
    ifstream fin(inFileName);
    int numFiles = 0;
    fin >> numFiles;
    for (int i = 1; i <= numFiles; i++) {
        string fileName;
        fin >> fileName;
        inFiles.push(make_pair(fileName, i));
    }
    numMap = numMappers;
    numReduce = numReducers;
    pthread_t threads[numMappers + numReducers];
    int arguments[numMappers + numReducers];
    pthread_barrier_init(&barrier, NULL, numMappers + numReducers);
    pthread_mutex_init(&list_file_mutex, NULL);
    pthread_mutex_init(&add_sol_mutex, NULL);
    int rc;
    for (char c = 'a'; c <= 'z'; c++) {
        alphabet.push(c);
    }
    for (int i = 0; i < numMappers + numReducers; i++) {
        arguments[i] = i;
        rc = pthread_create(&threads[i], NULL, threadFunction, &arguments[i]);
        if (rc) {
            cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
    }
    for (int i = 0; i < numMappers + numReducers; i++) {
        rc = pthread_join(threads[i], NULL);
        if (rc) {
            cout << "Error:unable to join," << rc << endl;
            exit(-1);
        }
    }
    return 0;
}