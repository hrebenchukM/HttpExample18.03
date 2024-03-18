#define _CRT_SECURE_NO_WARNINGS
#pragma comment (lib, "Ws2_32.lib")
#include <Winsock2.h>
#include <ws2tcpip.h>

#include <iostream>
#include <ctime>

#include <string>
using namespace std;



string findValue(const string& jsonStr, const string& key) {
    string value;

    int keyPos = jsonStr.find("\"" + key + "\":");
    //  (\" ) = "
    // ( + ... + ) = connection (concatenation)
    //(\":) = " :

    if (keyPos != -1) {

        int valueStart = jsonStr.find(':', keyPos);

        if (valueStart != -1) {

            valueStart++;

            int valueEnd = jsonStr.find_first_of(",}", valueStart);

            if (valueEnd != -1) {

                value = jsonStr.substr(valueStart, valueEnd - valueStart);
            }
        }
    }
    return value;
}

string fromIntToDataTime(int timestamp) {
    // Преобразование timestamp в структуру времени (необходимо использовать тип time_t)
    time_t rawtime = timestamp;
    return string(ctime(&rawtime));
}



int main()
{
    setlocale(0, "ru");

    ////////////////hw
    char inputCity[100]; // Буфер для хранения введенного города
    cout << " Input your city : ";
    cin.getline(inputCity, sizeof(inputCity));
    //////////////////hw


    //1. инициализация "Ws2_32.dll" для текущего процесса
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);

    int err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) {

        cout << "WSAStartup failed with error: " << err << endl;
        return 1;
    }  

    //инициализация структуры, для указания ip адреса и порта сервера с которым мы хотим соединиться
   
   

    char hostname[255] = "api.openweathermap.org";//не сокет а точка подключения к хосту
    
    addrinfo* result = NULL;    
    
    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    int iResult = getaddrinfo(hostname, "http", &hints, &result);
    if (iResult != 0) {
        cout << "getaddrinfo failed with error: " << iResult << endl;
        WSACleanup();
        return 3;
    }     

    SOCKET connectSocket = INVALID_SOCKET;
    addrinfo* ptr = NULL;

    //Пробуем присоединиться к полученному адресу
    for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

        //2. создание клиентского сокета
        connectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
        if (connectSocket == INVALID_SOCKET) {
            printf("socket failed with error: %ld\n", WSAGetLastError());
            WSACleanup();
            return 1;
        }

       //3. Соединяемся с сервером
        iResult = connect(connectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
        if (iResult == SOCKET_ERROR) {
            closesocket(connectSocket);
            connectSocket = INVALID_SOCKET;
            continue;
        }
        break;
    }

    //4. HTTP Request

   // string uri = "/data/2.5/weather?q=Odessa&appid=75f6e64d49db78658d09cb5ab201e483&mode=JSON&units=metric";
   
    
    
    ///////////hw
    string uri = "/data/2.5/weather?q=";
    uri = uri + inputCity;
    uri = uri + "&appid=75f6e64d49db78658d09cb5ab201e483&mode=JSON&units=metric";
    ///////////hw


    string request = "GET " + uri + " HTTP/1.1\n"; 
    request += "Host: " + string(hostname) + "\n";
    request += "Accept: */*\n";
    request += "Accept-Encoding: gzip, deflate, br\n";   
    request += "Connection: close\n";   
    request += "\n";

    //отправка сообщения
    if (send(connectSocket, request.c_str(), request.length(), 0) == SOCKET_ERROR) {
        cout << "send failed: " << WSAGetLastError() << endl;
        closesocket(connectSocket);
        WSACleanup();
        return 5;
    }
    cout << "send data" << endl;

    //5. HTTP Response

    string response;

    const size_t BUFFERSIZE = 1024;
    char resBuf[BUFFERSIZE];

    int respLength;

    do {
        respLength = recv(connectSocket, resBuf, BUFFERSIZE, 0);
        if (respLength > 0) {
            response += string(resBuf).substr(0, respLength);           
        }
        else {
            cout << "recv failed: " << WSAGetLastError() << endl;
            closesocket(connectSocket);
            WSACleanup();
            return 6;
        }

    } while (respLength == BUFFERSIZE);

   cout << response << endl;





    //////////hw


    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);

    string country = findValue(response, "country");
    string city = findValue(response, "name");
    string coordinatLon = findValue(response, "lon");
    string coordinatLat = findValue(response, "lat");

    string temperature = findValue(response, "temp");
    string sunrise = findValue(response, "sunrise");
    string sunset = findValue(response, "sunset");


    int sunriseUnixTime = stoi(sunrise);
    int sunsetUnixTime = stoi(sunset);
    string sunriseStr = fromIntToDataTime(sunriseUnixTime);
    string sunsetStr = fromIntToDataTime(sunsetUnixTime);

    SetConsoleTextAttribute(h, 6);
    cout << "Country: ";
    SetConsoleTextAttribute(h, 7);
    cout<< country << endl;
 
    SetConsoleTextAttribute(h, 6);
    cout << "City: ";
    SetConsoleTextAttribute(h, 7);
    cout<< city << endl;

    SetConsoleTextAttribute(h, 6);
    cout << "Coordinates: ";
    SetConsoleTextAttribute(h, 7);
    cout<<"lon=" << coordinatLon << " lat=" << coordinatLat << endl;
    

    SetConsoleTextAttribute(h, 6);
    cout << "Temperature: ";
    SetConsoleTextAttribute(h, 7);
    cout<< temperature << endl;
    

    SetConsoleTextAttribute(h, 6);
    cout << "Sunrise: ";
    SetConsoleTextAttribute(h, 7);
    cout<< sunriseStr << endl;
   

    SetConsoleTextAttribute(h, 6);
    cout << "Sunset: ";
    SetConsoleTextAttribute(h, 7);
    cout<< sunsetStr << endl;
   
    ///////////hw

   
   




    //отключает отправку и получение сообщений сокетом
    iResult = shutdown(connectSocket, SD_BOTH);
    if (iResult == SOCKET_ERROR) {
        cout << "shutdown failed: " << WSAGetLastError() << endl;
        closesocket(connectSocket);
        WSACleanup();
        return 7;
    }

    closesocket(connectSocket);
    WSACleanup();
}