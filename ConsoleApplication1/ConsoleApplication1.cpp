#include <windows.h>
#include <iostream>
#include <fstream>
#include <String>

/*
* [+] создает два потока, передает порядковый номер потока в потоковую функцию;
* [ ] выполняет в цикле в каждом потоке заданное количество операций и пишет в файл время выполнения каждой операции в миллисекундах
*/

struct ThreadData {
    int threadNum;
    int operationCount;
};

clock_t timeStart;
std::string output;

static double countTime(clock_t start, clock_t end)
{
    double time = static_cast<double>(end - start) * 1000.0 / CLOCKS_PER_SEC;
    return time;
}

static DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
    ThreadData* data = (ThreadData*)lpParam;
    int threadNum = data->threadNum;
    int operationCount = data->operationCount;

    for (int i = 0; i < operationCount; ++i)
    {   
        clock_t currentTime = clock();
        int k = 0;
        for (int i = 0; i < 1000000; ++i)
        {
            k += 1;
        }
        double time = countTime(timeStart, currentTime);
        int timeInt = static_cast<int>(time);
        output += std::to_string(threadNum) + "|" + std::to_string(timeInt) + "\n";
    }

    ExitThread(0);
}

int main(int argc, char* argv[])
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    int n = 2;
    int k = atoi(argv[1]);                      // количество операций в цикле

    std::string cat;

    std::cin >> cat;

    timeStart = clock();

    HANDLE* handles = new HANDLE[n];            //создание n потоков
    ThreadData* threadData = new ThreadData[n]; //массив для хранения данных потоков

    for (int i = 0; i < n; i++)
    {
        threadData[i].threadNum = i + 1;
        threadData[i].operationCount = k;

        handles[i] = CreateThread(NULL, 0, ThreadProc, &threadData[i], 0, NULL);
        if (i == 0) {
            SetThreadPriority(handles[i], THREAD_PRIORITY_ABOVE_NORMAL);
        }
        else {
            SetThreadPriority(handles[i], THREAD_PRIORITY_NORMAL);
        }
    }

    WaitForMultipleObjects(n, handles, TRUE, INFINITE);


    for (int i = 0; i < n; i++) {
        CloseHandle(handles[i]);
    }

    delete[] handles;
    delete[] threadData;


    std::ofstream file("output.txt");
    file << output;
    clock_t currentTime = clock();
    file.close();

    return 0;
}
