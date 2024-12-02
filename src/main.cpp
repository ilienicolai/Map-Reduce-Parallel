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
struct thread_data {
    queue<pair<string, int>> inFiles;
    vector<pair<string, int>> partialResult;
    pthread_barrier_t barrier;
    pthread_mutex_t list_file_mutex, add_sol_mutex;
    queue<char> alphabet;
    int numMap;
    int numReduce;
    int *thread_id;
};
struct thread_id {
    struct thread_data *data;
    int id;
};


void *threadFunction(void *arg) {
    struct thread_id *thread = (struct thread_id *)arg;
    int th_id = thread->id;
    thread_data *data = thread->data;
    cout << "Thread " << th_id << " started\n";
    if (th_id < thread->data->numMap) {
        while (true) {
            string fileName;
            int index;
            pthread_mutex_lock(&(data->list_file_mutex));
            if (data->inFiles.empty()) {
                pthread_mutex_unlock(&(data->list_file_mutex));
                break;
            }
            fileName = data->inFiles.front().first;
            index = data->inFiles.front().second;
            data->inFiles.pop();
            pthread_mutex_unlock(&(data->list_file_mutex));
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
            pthread_mutex_lock(&(data->add_sol_mutex));
            data->partialResult.insert(data->partialResult.end(), localResult.begin(), localResult.end());
            pthread_mutex_unlock(&(data->add_sol_mutex));
        }
    }
    pthread_barrier_wait(&(data->barrier));
    if (th_id >= data->numMap) {
        
        while (true) {
            map<string , set<int>> localResult;
            char letter;
            pthread_mutex_lock(&(data->list_file_mutex));
            if (data->alphabet.empty()) {
                pthread_mutex_unlock(&(data->list_file_mutex));
                break;
            }
            letter = data->alphabet.front();
            data->alphabet.pop();
            pthread_mutex_unlock(&(data->list_file_mutex));
            for (auto &it : data->partialResult) {
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
    struct thread_data data;
    data.numMap = numMappers;
    data.numReduce = numReducers;
    
    string inFileName = argv[3];
    ifstream fin(inFileName);
    int numFiles = 0;
    fin >> numFiles;
    for (int i = 1; i <= numFiles; i++) {
        string fileName;
        fin >> fileName;
        data.inFiles.push(make_pair(fileName, i));
    }
    pthread_t threads[numMappers + numReducers];
    int arguments[numMappers + numReducers];
    pthread_barrier_init(&data.barrier, NULL, numMappers + numReducers);
    pthread_mutex_init(&data.list_file_mutex, NULL);
    pthread_mutex_init(&data.add_sol_mutex, NULL);
    int rc;
    for (char c = 'a'; c <= 'z'; c++) {
        data.alphabet.push(c);
    }
    for (int i = 0; i < numMappers + numReducers; i++) {
        arguments[i] = i;
        struct thread_id *thread = new struct thread_id;
        thread->data = &data;
        thread->id = i;
        rc = pthread_create(&threads[i], NULL, threadFunction, thread);
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