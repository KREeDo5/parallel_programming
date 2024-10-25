#include <windows.h>
#include <iostream>
/*
* [+] создает два потока, передает порядковый номер потока в потоковую функцию;
* [ ] выполняет в цикле в каждом потоке заданное количество операций и пишет в файл время выполнения каждой операции в миллисекундах
*/

struct ThreadData {
    int threadNum;
    int operationCount;
};

clock_t timeStart;

static double countTime(clock_t start, clock_t end)
{
    double time = static_cast<double>(end - start) * 1000 / CLOCKS_PER_SEC;
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
        std::cout << threadNum << "|" << countTime(timeStart, currentTime) << std::endl;
    }

    ExitThread(0);
}

int main(int argc, char* argv[])
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    int n = 2;
    int k = atoi(argv[1]);                      // количество операций в цикле

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

    return 0;
}
