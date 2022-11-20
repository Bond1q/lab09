#include <iostream>
#include <windows.h>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

#define USERS 5
#define SIZE 50

using namespace std;

HANDLE hMutex;
HANDLE myH = CreateMutex(0, FALSE, (LPCWSTR)"Globa\\myH");
int counter = 0;

string prohibitedWords[SIZE];
int prohibitedAmount = 3;
struct pr {
    vector <string> words;
    string wordToCheck;
};
DWORD WINAPI MassageChecker(__in LPVOID params) {

    pr parameters = *(pr*)params;
    for (string word : parameters.words) {
        if (word.compare(parameters.wordToCheck) == 0) {
            WaitForSingleObject(myH, INFINITE);
            counter++;
            ReleaseMutex(myH);
        }
    }
    return 0;
}
int main(int argc, const char** argv) {


    // ira -------------------------------------------
    wcout << "Creating an instance of a named pipe..." << endl;

    HANDLE pipe = CreateNamedPipe(
        L"\\\\.\\pipe\\my_pipe",
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE,
        PIPE_UNLIMITED_INSTANCES,
        0,
        0,
        0,
        NULL
    );

    if (pipe == NULL || pipe == INVALID_HANDLE_VALUE) {
        wcout << "Failed to create outbound pipe instance.";
        system("pause");
        return 1;
    }

    wcout << "Waiting for a client to connect to the pipe..." << endl;
    for (int us = 0; us < USERS; us++) {
        BOOL result = ConnectNamedPipe(pipe, NULL);
        if (!result) {
            wcout << "Failed to make connection on named pipe." << endl;
            CloseHandle(pipe);
            system("pause");
            return 1;
        }

        wcout << "Reading name from client..." << endl;
        wchar_t buffer[128];
        DWORD numBytesRead = 0;
        result = ReadFile(
            pipe,
            buffer,
            127 * sizeof(wchar_t),
            &numBytesRead,
            NULL
        );
        const wchar_t* data = L"Your name successfully added";
        if (result) {
            buffer[numBytesRead / sizeof(wchar_t)] = '\0';
            wstring ws(buffer);
            string name(ws.begin(), ws.end());
            ifstream myfileRead;
            myfileRead.open("Users.txt");
            string fileData;
            string line;

            int index = 0;
            if (myfileRead.is_open())
            {
                getline(myfileRead, line);
                while (myfileRead)
                {
                    fileData += line;
                    getline(myfileRead, line);
                }
            }
            myfileRead.close();


            if (fileData.find(name) == std::string::npos) {
                ofstream myfile;
                myfile.open("Users.txt", ios_base::app);
                myfile << name + "\n";
                myfile.close();
            }
            else
                data = L"You are already registered:)";

        }
        else {
            wcout << "Failed to read name from the pipe." << endl;
        }
        wcout << "Sending data to pipe..." << endl;
        DWORD numBytesWritten = 0;
        result = WriteFile(
            pipe,
            data,
            wcslen(data) * sizeof(wchar_t),
            &numBytesWritten,
            NULL
        );

        if (result) {
            wcout << "Number of bytes sent: " << numBytesWritten << endl;
        }
        else {
            wcout << "Failed to send data." << endl;
        }
        // ira -------------------------------------------




        // misha -------------------------------------------
        ifstream wordsData;
        string line;
        string wordsStr = "";
        wordsData.open("words.txt");
        int wordsArrSize = 0;

        if (wordsData.is_open())
        {
            getline(wordsData, line);
            while (wordsData)
            {
                wordsStr += line + " ";
                getline(wordsData, line);
                wordsArrSize++;
            }
        }
        wordsData.close();
        line = "";

        //prohibitedWords = new string[wordsArrSize];

        int iter = 0;
        stringstream ssin(wordsStr);
        while (ssin.good() && iter < wordsArrSize) {
            ssin >> prohibitedWords[iter];
            ++iter;
        }

        // -------------------------------------------

        wcout << "Reading client message..." << endl;
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
            wcout << "Number of bytes read: " << numBytesRead << endl;
            wcout << "Message: " << buffer << endl;
        }
        else {
            wcout << "Failed to read data from the pipe." << endl;
        }
        wstring ws1(buffer);
        string message(ws1.begin(), ws1.end());
        vector<string> words{};
        string delimiter = " ";
        size_t pos = 0;
        string token;
        while ((pos = message.find(delimiter)) != string::npos) {
            token = message.substr(0, pos);
            words.push_back(token);
            message.erase(0, pos + delimiter.length());
        }
        words.push_back(message);

        HANDLE* h = new HANDLE[sizeof(prohibitedWords) / sizeof(*prohibitedWords)];
        pr* par = new pr[sizeof(prohibitedWords) / sizeof(*prohibitedWords)];
        int i = 0;
        counter = 0;
        for (string w : prohibitedWords) {
            DWORD threadID;
            par[i].words = words;
            par[i].wordToCheck = w;
            h[i] = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MassageChecker, (LPVOID)&par[i++], NULL, &threadID);
        }
        WaitForMultipleObjects(sizeof(prohibitedWords) / sizeof(*prohibitedWords), h, FALSE, INFINITE);


        wcout << "Sending data to pipe..." << endl;
        if (counter < prohibitedAmount)
            data = buffer;
        else
            data = L"Too many prohibited words in your message!";

        numBytesWritten = 0;
        result = WriteFile(
            pipe,
            data,
            wcslen(data) * sizeof(wchar_t),
            &numBytesWritten,
            NULL
        );

        if (result) {
            wcout << "Number of bytes sent: " << numBytesWritten << endl;
        }
        else {
            wcout << "Failed to send data." << endl;
        }
        if (!DisconnectNamedPipe(pipe))
        {
            printf("Disconnect failed %d\n", GetLastError());
        }
        else
        {
            printf("Disconnect successful\n");
        }

        // misha -------------------------------------------

    }
    CloseHandle(pipe);

    wcout << "Done." << endl;

    system("pause");
    return 0;
}
