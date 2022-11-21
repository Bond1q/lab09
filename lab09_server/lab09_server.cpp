#include <iostream>
#include <windows.h>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

#define SIZE 50
#define USERS 5
// крч можна придумати вихід з циклу (вайл тру)
// бо #define USERS 5 не файно

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
    wcout << "Creating a instance of a pipe..." << "\n";

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

    BOOL result = pipe != NULL && pipe != INVALID_HANDLE_VALUE;
    if (!result) {
        wcout << "Failed in a pipe creating.";
        return 1;
    }

   // while (true) 
    for(int u = 0; u < USERS; u++)
    {
        wcout << "Waiting for users..." << "\n";
        result = ConnectNamedPipe(
            pipe,
            NULL
        );
        if (!result) {
            wcout << "Failed to connect to pipe." << "\n";
            CloseHandle(pipe);
            return 1;
        }

        wcout << "Reading user name..." << "\n";
        char buffer[128];
        DWORD bytesRead = 0;
        result = ReadFile(
            pipe,
            buffer,
            127,
            &bytesRead,
            NULL
        );
        if (!result) {
            wcout << "Failed to read name from the pipe." << "\n";
            CloseHandle(pipe);
            return 1;
        }

        wcout << "Number of read bytes: " << bytesRead << "\n";

        buffer[bytesRead / sizeof(char)] = '\0';
        string ws(buffer);
        string name(ws.begin(), ws.end());
        ifstream usersFile;
        usersFile.open("Users.txt");

        string fileData;
        string bData = "Welcome";

        int index = 0;
        if (usersFile.is_open())
        {
            bool found = false;
            string line;
            getline(usersFile, line);
            while (usersFile)
            {
                if (line == name) {
                    bData += " back";
                    found = true;
                    break;
                }
                getline(usersFile, line);
            }
            usersFile.close();

            if (!found) {
                ofstream usersFile;
                usersFile.open("Users.txt", ios_base::app);
                usersFile << name + "\n";
                usersFile.close();
            }
        }

        bData += ", " + name;
        

        cout << "Sending data to pipe..." << endl;
        DWORD bytesWritten = 0;
        result = WriteFile(
            pipe,
            bData.c_str(),
            strlen(bData.c_str()),
            &bytesWritten,
            NULL
        );

        if (result) 
            cout << "Number of written bytes: " << bytesWritten << "\n";
        else 
            cout << "Failed to send data." << "\n";
       
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

        cout << "Reading client message..." << endl;
        buffer[128];
        bytesRead = 0;
        result = ReadFile(
            pipe,
            buffer,
            127,
            &bytesRead,
            NULL
        );

        if (result) {
            buffer[bytesRead / sizeof(char)] = '\0';
            cout << "Number of bytes read: " << bytesRead << endl;
            cout << "Message: " << buffer << endl;
        }
        else {
            cout << "Failed to read data from the pipe." << endl;
        }
        string ws1(buffer);
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


        cout << "Sending data to pipe..." << endl;
        if (counter < prohibitedAmount)
            bData = buffer;
        else
            bData = "Too many prohibited words in your message!";

        bytesWritten = 0;
        result = WriteFile(
            pipe,
            bData.c_str(),
            strlen(bData.c_str()),
            &bytesWritten,
            NULL
        );

        if (result) {
            wcout << "Number of bytes sent: " << bytesWritten << endl;
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
