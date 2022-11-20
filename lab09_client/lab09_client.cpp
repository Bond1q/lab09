#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include <tchar.h>
using namespace std;
int main()
{
    HANDLE pipe;
    while (true) {
        pipe = CreateFile(
            L"\\\\.\\pipe\\my_pipe",
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
        );

        if (pipe == INVALID_HANDLE_VALUE)
            break;
        if (GetLastError() == ERROR_PIPE_BUSY)
        {
            if (!WaitNamedPipe(L"\\\\.\\pipe\\my_pipe", NMPWAIT_USE_DEFAULT_WAIT))
                continue;
        }
        else
            break;
        wcout << "Failed to connect to pipe." << endl;

        system("pause");
        return 1;

    }
    // Sending name to server 
    wcout << "Enter your name:" << endl;
    string name;
    getline(cin, name);
    wstring widestr = wstring(name.begin(), name.end());
    const wchar_t* data = widestr.c_str();
    DWORD numBytesWritten = 0;
    BOOL result = WriteFile(
        pipe,
        data,
        wcslen(data) * sizeof(wchar_t),
        &numBytesWritten,
        NULL
    );

    wchar_t buffer[128];
    DWORD numBytesRead = 0;
    result = ReadFile(
        pipe,
        buffer,
        127 * sizeof(wchar_t),
        &numBytesRead,
        NULL
    );

    if (result) {
        buffer[numBytesRead / sizeof(wchar_t)] = '\0';
        wcout << buffer << endl;
    }
    else {
        wcout << "Failed to read data from the pipe." << endl;
    }

    wcout << "Enter your message:" << endl;
    string data1;
    getline(cin, data1);
    widestr = wstring(data1.begin(), data1.end());
    data = widestr.c_str();
    numBytesWritten = 0;
    result = WriteFile(
        pipe,
        data,
        wcslen(data) * sizeof(wchar_t),
        &numBytesWritten,
        NULL
    );

    buffer[128];
    numBytesRead = 0;
    result = ReadFile(
        pipe,
        buffer,
        127 * sizeof(wchar_t),
        &numBytesRead,
        NULL
    );
    if (result) {
        buffer[numBytesRead / sizeof(wchar_t)] = '\0';
        wcout << buffer << endl;
    }
    else {
        wcout << "Failed to read data from the pipe." << endl;
    }
    CloseHandle(pipe);
    wcout << "Done." << endl;
    system("pause");
    return 0;
}