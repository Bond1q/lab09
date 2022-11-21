#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
using namespace std;

bool connectToServer(HANDLE &pipe);

int main()
{
    HANDLE pipe = NULL;
    bool isConnected = connectToServer(pipe);
    if (!isConnected) return 1;

    cout << "Hello, you are on forum!\nPlease enter your name:" << endl;
    string name;
    getline(cin, name);
    DWORD numBytesWritten = 0;
    BOOL result = WriteFile(
        pipe,
        name.c_str(),
        strlen(name.c_str()),
        &numBytesWritten,
        NULL
    );

    char serverText[1000];
    string data;
    DWORD numBytesRead = 0;
    result = ReadFile(
        pipe,
        serverText,
        1000,
        &numBytesRead,
        NULL
    );

    if (result) {
        serverText[numBytesRead] = '\0';
        cout << serverText << endl;
    }
    else {
        cout << "Cannot get data from server." << endl;
    }


    cout << "Enter your message:" << endl;
    string message;
    getline(cin, message);
    numBytesWritten = 0;
    result = WriteFile(
        pipe,
        message.c_str(),
        strlen(message.c_str()),
        &numBytesWritten,
        NULL
    );

    numBytesRead = 0;
    result = ReadFile(
        pipe,
        serverText,
        1000,
        &numBytesRead,
        NULL
    );
    if (result) {
        serverText[numBytesRead] = '\0';
        cout << serverText << endl;
    }
    else {
        cout << "Cannot get data from server." << endl;
    }
    CloseHandle(pipe);
    cout << "Done." << endl;
    system("pause");
    return 0;
}

bool connectToServer(HANDLE &pipe) {
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
        cout << "Failed to connect to the server." << endl;

        system("pause");
        return false;
    }
    return true;
}
