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
    // coada de fisiere de intrare cu id-ul lor
    queue<pair<string, int>> inFiles;
    // rezultatul partial dupa map
    vector<pair<string, int>> partialResult;
    // bariera
    pthread_barrier_t barrier;
    // mutex-uri pentru coada de fisiere si rezultatul partial
    pthread_mutex_t queue_mutex, add_sol_mutex;
    // coada de litere
    queue<char> alphabet;
    // numarul de mapperi si reduceri
    int numMap;
    int numReduce;
};
struct thread_id {
    // datele programului
    struct thread_data *data;
    // id-ul thread-ului
    int id;
};


void *threadFunction(void *arg) {
    struct thread_id *thread = (struct thread_id *)arg;
    int th_id = thread->id;
    thread_data *data = thread->data;
    // thread-ul este un mapper
    if (th_id < thread->data->numMap) {
        while (true) {
            // extractia unui fisier din coada
            string fileName;
            int index;
            pthread_mutex_lock(&(data->queue_mutex));
            if (data->inFiles.empty()) {
                pthread_mutex_unlock(&(data->queue_mutex));
                break;
            }
            fileName = data->inFiles.front().first;
            index = data->inFiles.front().second;
            data->inFiles.pop();
            pthread_mutex_unlock(&(data->queue_mutex));
            // citire cuvant cu cuvant din fisier
            ifstream fin(fileName);
            string word;
            vector<pair<string, int>> localResult;
            while (fin >> word) {
                // determinarea cuvantului fara caractere speciale
                string word_to_analize;
                for (char c : word) {
                    if (std::isalpha(c)) {
                        word_to_analize += string(1,tolower(c));
                    }
                }
                // adaugarea cuvantului in rezultatul partial
                if (word_to_analize.size() > 0) {
                    localResult.push_back(make_pair(word_to_analize, index));
                }
            }
            // adaugarea rezultatului partial 
            // in vectorul de rezultate partiale dupa map
            pthread_mutex_lock(&(data->add_sol_mutex));
            data->partialResult.insert(data->partialResult.end(), 
                                localResult.begin(), localResult.end());
            pthread_mutex_unlock(&(data->add_sol_mutex));
        }
    }
    // astetarea thread-urilor mapperi
    pthread_barrier_wait(&(data->barrier));
    // thread-ul este un reducer
    if (th_id >= data->numMap) {
        
        while (true) {
            // rezultatul local al reducerului
            map<string , set<int>> localResult;
            // litera pe care o proceseaza reducerul
            char letter;
            // extragerea literei din coada
            pthread_mutex_lock(&(data->queue_mutex));
            if (data->alphabet.empty()) {
                pthread_mutex_unlock(&(data->queue_mutex));
                break;
            }
            letter = data->alphabet.front();
            data->alphabet.pop();
            pthread_mutex_unlock(&(data->queue_mutex));
            // adaugarea in rezultatul local a cuvintelor care incep cu litera letter
            for (auto &it : data->partialResult) {
                if (it.first[0] == letter) {
                    localResult[it.first].insert(it.second);
                }
            }
            // mutarea rezultatului local in vector 
            // ca sa fie sortat si scris in fisier
            vector<pair<string, set<int>>> sortedResult;
            map<string, set<int>>::iterator it;
            for (it = localResult.begin(); it != localResult.end(); it++) {
                sortedResult.push_back(make_pair(it->first, it->second));
            }
            // sortarea rezultatului local
            sort(sortedResult.begin(), sortedResult.end(), [](const pair<string, set<int>> &a, const pair<string, set<int>> &b) {
                if (a.second.size() == b.second.size()) {
                    return a.first < b.first;
                }
                return a.second.size() > b.second.size();
            });
            // scrierea rezultatului in fisier
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
    if (argc < 4) {
        cout << "Utilizati ./tema1 <numar_mapperi> <numar_reduceri> <fisier_intrare>";
        return -1;
    }
    // numarul de mapperi
    int numMappers = std::stoi(argv[1]);
    // numarul de reduceri
    int numReducers = std::stoi(argv[2]);
    // datele care vor fi trimise thread-urilor
    struct thread_data data;
    data.numMap = numMappers;
    data.numReduce = numReducers;
    // fisierul de intrare
    string inFileName = argv[3];
    ifstream fin(inFileName);
    int numFiles = 0;
    fin >> numFiles;
    // citirea numelor fisierelor si indicii lor
    // in coada inFiles
    for (int i = 1; i <= numFiles; i++) {
        string fileName;
        fin >> fileName;
        data.inFiles.push(make_pair(fileName, i));
    }
    // thread-urile
    pthread_t threads[numMappers + numReducers];
    // initializare elemente de sincronizare
    pthread_barrier_init(&data.barrier, NULL, numMappers + numReducers);
    pthread_mutex_init(&data.queue_mutex, NULL);
    pthread_mutex_init(&data.add_sol_mutex, NULL);
    int rc;
    // initializare coada de litere
    for (char c = 'a'; c <= 'z'; c++) {
        data.alphabet.push(c);
    }
    // crearea thread-urilor
    for (int i = 0; i < numMappers + numReducers; i++) {
        // creare structura ce contine id-ul si datele programului
        struct thread_id *thread = new struct thread_id;
        thread->data = &data;
        thread->id = i;
        rc = pthread_create(&threads[i], NULL, threadFunction, thread);
        if (rc) {
            cout << "Error:unable to create thread," << rc << endl;
            exit(-1);
        }
    }
    // asteptarea thread-urilor
    for (int i = 0; i < numMappers + numReducers; i++) {
        rc = pthread_join(threads[i], NULL);
        if (rc) {
            cout << "Error:unable to join," << rc << endl;
            exit(-1);
        }
    }
    return 0;
}