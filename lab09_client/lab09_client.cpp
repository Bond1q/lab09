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
        cout << "Failed to connect to th." << endl;

        system("pause");
        return 1;

    }
    // Sending name to server 
    cout << "Hello, you are on forum!\nPlease enter your name:" << endl;
    string name;
    getline(cin, name);
    //wstring widestr = wstring(name.begin(), name.end());
    //const wchar_t* data = widestr.c_str();
    DWORD numBytesWritten = 0;
    BOOL result = WriteFile(
        pipe,
        name.c_str(),
        strlen(name.c_str()),

        &numBytesWritten,
        NULL
    );

    char buffer[1000];
    string data;
    DWORD numBytesRead = 0;
    result = ReadFile(
        pipe,
        buffer,
        1000,
        &numBytesRead,
        NULL
    );

    if (result) {
        buffer[numBytesRead] = '\0';
        cout << buffer << endl;
    }
    else {
        cout << "Cannot get data from server." << endl;
    }


    cout << "Enter your message:" << endl;
    string message;
    getline(cin, message);
    //widestr = wstring(message.begin(), message.end());
    //data = widestr.c_str();
    numBytesWritten = 0;
    result = WriteFile(
        pipe,
        message.c_str(),
        strlen(message.c_str()),
        &numBytesWritten,
        NULL
    );

    //buffer[128];
    numBytesRead = 0;
    result = ReadFile(
        pipe,
        buffer,
        1000,
        &numBytesRead,
        NULL
    );
    if (result) {
        buffer[numBytesRead] = '\0';
        cout << buffer << endl;
    }
    else {
        cout << "Cannot get data from server." << endl;
    }
    CloseHandle(pipe);
    cout << "Done." << endl;
    system("pause");
    return 0;
}