#include <windows.h>
#include <iostream>
/*
[+] Реализовать консольное приложение на С++, для работы с потоками использовать функцию CreateThread (Windows SDK).
[+] Количество потоков передавать приложению в командной строке.
[] Приложение должно ожидать окончание работы всех потоков после чего корректно завершаться. Для ожидания окончания работы все потоков используйте функцию WaitForMultipleObjects (Windows SDK).
[] Разработать и сдать программу на занятии.
*/

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
    int threadNum = *(int*)lpParam;

    std::cout << "Поток №" << threadNum << " выполняет свою работу\n";

    ExitThread(0); // функция устанавливает код завершения потока в 0
}

int main(int argc, char* argv[])
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    int n = atoi(argv[1]);

    if (n <= 0) {
        return 1;
    }

    // создание n потоков
    HANDLE* handles = new HANDLE[n];

    int* threadIds = new int[n];    // создание массива для хранения номеров потоков

    for (int i = 0; i < n; i++)
    {
        threadIds[i] = i + 1;
        handles[i] = CreateThread(NULL, 0, ThreadProc, &threadIds[i], 0, NULL);
    }

    // ожидание окончания работы n потоков
    WaitForMultipleObjects(n, handles, TRUE, INFINITE);

    return 0;
}
