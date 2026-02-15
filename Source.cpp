#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cstdio>  
#include <string>

using namespace std;

// -------------------- настройки --------------------
const char* INPUT_FILE = "big.txt";
const char* OUTPUT_FILE = "sorted.txt";

// chunkSize = сколько чисел читаем в пам€ть за раз
const int CHUNK_SIZE = 1'000'000;

// —колько чисел сгенерировать
const long long GENERATE_COUNT = 5'000'000;

// -------------------- утилиты имен файлов --------------------
string runFileName(int idx) {
    return "run_" + to_string(idx) + ".txt";
}

string mergeFileName(int pass, int idx) {
    return "merge_" + to_string(pass) + "_" + to_string(idx) + ".txt";
}

// -------------------- генераци€ большого файла --------------------
void generateBigFile(const char* filename, long long count) {
    ofstream out(filename, ios::out | ios::trunc);
    if (!out) {
        throw runtime_error("Cannot open file for generation");
    }

    srand((unsigned)time(nullptr));
    for (long long i = 0; i < count; ++i) {
        int x = rand();
        out << x << "\n";
    }
}

// -------------------- шаг 1: режем на куски и сортируем каждый --------------------
int makeSortedRuns(const char* inputFile, int chunkSize) {
    ifstream in(inputFile);
    if (!in) throw runtime_error("Cannot open input file");

    int* arr = new int[chunkSize];
    int runCount = 0;

    while (!in.eof()) {
        int n = 0;

        // читаем chunk
        while (n < chunkSize && (in >> arr[n])) {
            n++;
        }

        if (n == 0) break; // больше нечего читать

        // сортируем chunk в пам€ти
        sort(arr, arr + n);

        // пишем во временный файл
        string rname = runFileName(runCount);
        ofstream out(rname.c_str(), ios::out | ios::trunc);
        if (!out) {
            delete[] arr;
            throw runtime_error("Cannot create run file");
        }
        for (int i = 0; i < n; ++i) out << arr[i] << "\n";

        runCount++;
    }

    delete[] arr;
    return runCount;
}

// -------------------- сли€ние двух отсортированных файлов --------------------
void mergeTwoFiles(const string& f1, const string& f2, const string& outFile) {
    ifstream a(f1.c_str());
    ifstream b(f2.c_str());
    ofstream out(outFile.c_str(), ios::out | ios::trunc);

    if (!a || !b || !out) throw runtime_error("Cannot open files for merge");

    int x, y;
    bool hasX = (bool)(a >> x);
    bool hasY = (bool)(b >> y);

    while (hasX && hasY) {
        if (x <= y) {
            out << x << "\n";
            hasX = (bool)(a >> x);
        }
        else {
            out << y << "\n";
            hasY = (bool)(b >> y);
        }
    }

    while (hasX) {
        out << x << "\n";
        hasX = (bool)(a >> x);
    }
    while (hasY) {
        out << y << "\n";
        hasY = (bool)(b >> y);
    }
}

// -------------------- шаг 2: объедин€ем ¬—≈ run-файлы попарно --------------------
string mergeAllRuns(int runCount) {
    if (runCount <= 0) throw runtime_error("No runs created");
    if (runCount == 1) return runFileName(0);

    int pass = 0;

    // текущие входные файлы Ч run_*.txt
    // на каждом проходе делаем merge_pass_i.txt
    while (runCount > 1) {
        int newCount = 0;
        int i = 0;

        while (i < runCount) {
            string outName = mergeFileName(pass, newCount);

            if (i + 1 < runCount) {
                // есть пара
                string f1 = (pass == 0) ? runFileName(i) : mergeFileName(pass - 1, i);
                string f2 = (pass == 0) ? runFileName(i + 1) : mergeFileName(pass - 1, i + 1);

                mergeTwoFiles(f1, f2, outName);

                // удал€ем входные файлы после успешного сли€ни€
                remove(f1.c_str());
                remove(f2.c_str());

                i += 2;
            }
            else {
                // непарный файл Ч просто переносим дальше без сли€ни€
                string last = (pass == 0) ? runFileName(i) : mergeFileName(pass - 1, i);

                // перенос (rename) в новый файл
                if (rename(last.c_str(), outName.c_str()) != 0) {
                    // запасной вариант: копирование содержимого
                    ifstream src(last.c_str());
                    ofstream dst(outName.c_str(), ios::out | ios::trunc);
                    int val;
                    while (src >> val) dst << val << "\n";
                    src.close();
                    dst.close();
                    remove(last.c_str());
                }

                i += 1;
            }

            newCount++;
        }

        runCount = newCount;
        pass++;
    }

    // осталс€ один файл
    return mergeFileName(pass - 1, 0);
}

// -------------------- main --------------------
int main() {
    try {
        cout << "1) Generating file...\n";
        generateBigFile(INPUT_FILE, GENERATE_COUNT);

        cout << "2) Creating sorted runs...\n";
        int runCount = makeSortedRuns(INPUT_FILE, CHUNK_SIZE);
        cout << "Runs created: " << runCount << "\n";

        cout << "3) Merging all runs...\n";
        string finalFile = mergeAllRuns(runCount);

        // переименуем итог в sorted.txt
        remove(OUTPUT_FILE); // если был
        if (rename(finalFile.c_str(), OUTPUT_FILE) != 0) {
            // если rename не удалось Ч копируем
            ifstream src(finalFile.c_str());
            ofstream dst(OUTPUT_FILE, ios::out | ios::trunc);
            int v;
            while (src >> v) dst << v << "\n";
            src.close();
            dst.close();
            remove(finalFile.c_str());
        }

        cout << "DONE. Output file: " << OUTPUT_FILE << "\n";
        cout << "Note: INPUT_FILE remains: " << INPUT_FILE << "\n";
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}
