#include <iostream>
#include <windows.h>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

#define USERS 5
#define PROHIBITED_AMOUNT 3

using namespace std;

HANDLE hMutex = CreateMutex(0, FALSE, (LPCWSTR)"Globa\\myH");
int counter = 0;

vector <string> prohibitedWords;

struct param {
    vector <string> words;
    string wordToCheck;
};

DWORD WINAPI MassageReview(__in LPVOID params) {

    param parameters = *(param*)params;
    for (string word : parameters.words) {
        if (word.compare(parameters.wordToCheck) == 0) {
            WaitForSingleObject(hMutex, INFINITE);
            counter++;
            ReleaseMutex(hMutex);
        }
    }
    return 0;
}

int main(int argc, const char** argv) {


    // ira -------------------------------------------
    cout << "Creating a instance of a pipe..." << "\n";

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
        cout << "Failed in a pipe creating.";
        return 1;
    }
    for(int u = 0; u < USERS; u++)
    {
        cout << "Waiting for users..." << "\n";
        result = ConnectNamedPipe(
            pipe,
            NULL
        );
        if (!result) {
            cout << "Failed to connect to pipe." << "\n";
            CloseHandle(pipe);
            return 1;
        }

        cout << "Reading user name..." << "\n";
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
            cout << "Failed to read name from the pipe." << "\n";
            CloseHandle(pipe);
            return 1;
        }

        cout << "Number of read bytes: " << bytesRead << "\n";

        buffer[bytesRead] = '\0';
        string name(buffer);
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

        cout << "Sending data to pipe..." << "\n";
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

        int iter = 0;
        stringstream ssin(wordsStr);

        string ssinBufStr;
        while (ssin.good() && iter < wordsArrSize) {
            ssin >> ssinBufStr;
            prohibitedWords.push_back(ssinBufStr);
            ssinBufStr = "";
            ++iter;
        }

        cout << "Reading message from client..." << endl;
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
            buffer[bytesRead] = '\0';
            cout << "Number of read bytes: " << bytesRead << endl;
            cout << "Message: " << buffer << endl;
        }
        else {
            cout << "Failed to read data from the pipe." << endl;
        }
        string message(buffer);
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


        HANDLE* h = new HANDLE[prohibitedWords.size()];
        param* par = new param[prohibitedWords.size()];
        int i = 0;
        counter = 0;
        for (string w : prohibitedWords) {
            DWORD threadID;
            par[i].words = words;
            par[i].wordToCheck = w;
            h[i] = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MassageReview, (LPVOID)&par[i++], NULL, &threadID);
        }
        WaitForMultipleObjects(prohibitedWords.size(), h, FALSE, INFINITE);


        cout << "Sending data to pipe..." << endl;
        if (counter < PROHIBITED_AMOUNT)
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
            cout << "Number of bytes sent: " << bytesWritten << endl;
        }
        else {
            cout << "Failed to send data." << endl;
        }
        if (!DisconnectNamedPipe(pipe))
        {
            cout << "Disconnect failed %d " << GetLastError() << endl;
        }
        else
        {
            cout <<  "Disconnect successful" << endl;
        }

        // misha -------------------------------------------




    }
    CloseHandle(pipe);
    cout << "Done." << endl;

    system("pause");
    return 0;
}
