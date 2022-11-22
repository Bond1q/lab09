#include <iostream>
#include <windows.h>
#include <fstream>
#include <vector>
#include <string>
#include <tchar.h>
#include <strsafe.h>
#include <sstream>


#define PROHIBITED_AMOUNT 3
#define BUFSIZE 512

using namespace std;

HANDLE hMutex = CreateMutex(0, FALSE, (LPCWSTR)"mutex");
 
int counter = 0;

vector <string> prohibitedWords;

struct param {
    vector <string> words;
    string wordToCheck;
};

struct paramCl {
    HANDLE pipe;
    int id;
};

void updateColors(int id) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), id+2);
}

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

DWORD WINAPI ClientThread(__in LPVOID lpvParam)
{

    paramCl p = *(paramCl*)lpvParam;
    HANDLE hPipe = p.pipe;
    updateColors(p.id);

    cout << "ClientThread created" << "\n";
    cout << "Reading user name..." << "\n";

    char buffer[BUFSIZE];
    DWORD bytesRead = 0;
    BOOL result = ReadFile(
            hPipe,
            buffer,
            BUFSIZE-1,
            &bytesRead,
            NULL
    );

     if (!result) {
         cout << "Failed to read name from the pipe." << "\n";
         CloseHandle(hPipe);
         return 1;
     }
     buffer[bytesRead] = '\0';
     string name(buffer);
     ifstream usersFile;
     usersFile.open("Users.txt");
     string fileData;

     updateColors(p.id);
     cout << "Number of read bytes: " << bytesRead << "\n";
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
         hPipe,
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

     iter = 0;

     cout << "Reading message from client..." << endl;
     buffer[BUFSIZE];
     bytesRead = 0;
     result = ReadFile(
         hPipe,
         buffer,
         BUFSIZE-1,
         &bytesRead,
         NULL
     );

     updateColors(p.id);
     if (result) {
         buffer[bytesRead] = '\0';
         cout << "Number of read bytes: " << bytesRead << endl;
         cout << "Message: " << buffer << endl;
     }
     else {
         cout << "Failed to read data from the pipe." << endl;
     }


     vector<string> words{};


     ssin.str(buffer);
     wordsArrSize = 0;

     for (int i = 0; i < bytesRead; i++)
         if (buffer[i] == ' ')
         {
             ++wordsArrSize;
         }

     while (ssin.good() && iter < wordsArrSize) {
         ssin >> ssinBufStr;
         words.push_back(ssinBufStr);
         ssinBufStr = "";
         ++iter;
     }



     HANDLE* h = new HANDLE[prohibitedWords.size()];
     param* par = new param[prohibitedWords.size()];
     int i = 0;
     counter = 0;
     for (string w : prohibitedWords) {
         DWORD threadID;
         par[i].words = words;
         par[i].wordToCheck = w;
         h[i] = CreateThread(
             0,
               0,  
             (LPTHREAD_START_ROUTINE)MassageReview,
             (LPVOID)&par[i++],
             NULL,
             &threadID);
     }
     WaitForMultipleObjects(prohibitedWords.size(), h, FALSE, INFINITE);


     cout << "Sending data to pipe..." << endl;
     if (counter < PROHIBITED_AMOUNT)
         bData = buffer;
     else
         bData = "Too many prohibited words in your message!";

     bytesWritten = 0;
     result = WriteFile(
         hPipe,
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
     if (!DisconnectNamedPipe(hPipe))
     {
         cout << "Disconnect failed" << GetLastError() << endl;
     }
     else
     {
         cout << "Disconnect successful" << endl;
     }

     // misha -------------------------------------------

     FlushFileBuffers(hPipe);
     DisconnectNamedPipe(hPipe);
     CloseHandle(hPipe);

     cout << "Client thread exiting." << endl;
     return 1;
}


int main(int argc, const char** argv) {

    int q = 2;

    DWORD  dwThreadId = 0;
    HANDLE  hThread = NULL;
    HANDLE pipe;

    cout << "Server started" << endl;


    int client = 0;
    while(1)
    {
        // ira -------------------------------------------  

        pipe = CreateNamedPipe(
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

        result = ConnectNamedPipe(pipe, NULL) ?
            TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

        if (result)
        {
            paramCl par;
            par.pipe = pipe;
            par.id = client;
            updateColors(client);
            cout<< "\n" <<"Client connected, creating a processing thread."<<"\n";
            hThread = CreateThread(
                NULL,              
                0,                 
                ClientThread,    
                &par,     
                0,                
                &dwThreadId);    

            if (hThread == NULL)
            {
                return -1;
            }
            else CloseHandle(hThread);
        }
        else
            CloseHandle(pipe);
        client++;
    }

    system("pause");
    return 0;
}
