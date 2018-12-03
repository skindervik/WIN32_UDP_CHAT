#define WIN32_LEAN_AND_MEAN 
#pragma warning(disable:4996)
#include <WS2tcpip.h>
#include <windows.h>
#include <string>
#include <thread>

#pragma comment (lib, "ws2_32.lib")

HWND chat_field = nullptr;
HWND send_field = nullptr;
HWND send_button = nullptr;
HWND IP_field = nullptr;

WSADATA data;
WORD version = MAKEWORD(2, 2);
sockaddr_in server;
SOCKET out;
SOCKET in;

std::string IP = "127.0.0.1";
int PORT = 54000;
int server_PORT = 54000;

std::string get_window_text(HWND name) {
	int len = GetWindowTextLength(name) + 1;
	char* text = new char[len];
	GetWindowText(name, text, len);
	return (std::string)text;
}

std::string get_time() {
	time_t rawtime;
	struct tm * timeinfo;
	char buffer[88];

	time(&rawtime);
	timeinfo = localtime(&rawtime);

	strftime(buffer, sizeof(buffer), "-%d-%m-%Y %H:%M:%S-", timeinfo);

	return buffer;
}

void display_msg(std::string msg) {
	SendMessage(chat_field, EM_SETSEL, 0, -1);
	SendMessage(chat_field, EM_SETSEL, -1, -1);
	SendMessage(chat_field, EM_REPLACESEL, TRUE, (LPARAM)(msg + "\n").c_str());
}

void start_server() {
	in = socket(AF_INET, SOCK_DGRAM, 0);

	sockaddr_in serverHint;
	serverHint.sin_addr.S_un.S_addr = ADDR_ANY; // Us any IP address available on the machine
	serverHint.sin_family = AF_INET; // Address format is IPv4
	serverHint.sin_port = htons(server_PORT); // Convert from little to big endian

	if (bind(in, (sockaddr*)&serverHint, sizeof(serverHint)) == SOCKET_ERROR)
	{
		display_msg("ERROR BINDING THE SOCKET");
	}

	sockaddr_in client; // Use to hold the client information (port / ip address)

	int clientLength = sizeof(client); // The size of the client information

	char buf[INT16_MAX];

	int failed_times = 5;

	while (true)
	{
		ZeroMemory(&client, clientLength); // Clear the client structure
		ZeroMemory(buf, 1024); // Clear the receive buffer

		int bytesIn = recvfrom(in, buf, 1024, 0, (sockaddr*)&client, &clientLength);
		if (bytesIn == SOCKET_ERROR)
		{
			if (failed_times == 5) {
				display_msg("ERROR RECEIVING FROM CLIANT");
				failed_times = 0;
			}

			failed_times++;

			using namespace std::chrono_literals;

			std::this_thread::sleep_for(1s);

			continue;
		}
		char clientIp[256]; // Create enough space to convert the address byte array
		ZeroMemory(clientIp, 256); // to string of characters

		inet_ntop(AF_INET, &client.sin_addr, clientIp, 256);

		display_msg((std::string)"MSG RECV. at " + get_time() + " from- " + clientIp + ": " + buf);

	}
}



void send_msg() {
	if (get_window_text(IP_field) != IP) {
		IP = get_window_text(IP_field);
		inet_pton(AF_INET, IP.c_str(), &server.sin_addr); // Convert from string to byte array
	}

	std::string s = get_window_text(send_field);

	display_msg( (std::string)"MSG SENT. at " + get_time() + " to- " + get_window_text(IP_field) + ": " + s );
	
	int sendOk = sendto(out, s.c_str(), s.size() + 1, 0, (sockaddr*)&server, sizeof(server));
	if (sendOk == SOCKET_ERROR)
	{
		display_msg("ERROR TRYING TO SEND");
	}

	SetWindowText(send_field,"");
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) { //Window message processing 

	switch (msg)
	{
	case WM_SIZE:
	{
		RECT rect;
		if (GetWindowRect(hwnd, &rect))
		{
			int width = rect.right - rect.left;
			int height = rect.bottom - rect.top;

			SetWindowPos(chat_field, 0, 0, 23, width-17, height-65-23, NULL);
			SetWindowPos(send_field, 0, 0, height-65, width - 68, 40, NULL);
			SetWindowPos(send_button, 0, width-66, height-62, 50, 20, NULL);
		}
	}
		break;
	case WM_COMMAND:
		
		switch (LOWORD(wParam)){

		case 2:{

			if (wParam == 83951618) {
				send_msg();
			}
		}
		break;

		case 1:{
			send_msg();
		}
		break;
		}
		break;
	case WM_CREATE: {

		CreateWindow("STATIC", "SEND TO: ", WS_VISIBLE | WS_CHILD , 2, 3, 70, 17, hwnd, nullptr, nullptr, nullptr);

		IP_field = CreateWindow("EDIT", IP.c_str(), WS_VISIBLE | WS_CHILD | WS_BORDER, 70, 3, 200, 17, hwnd, nullptr, nullptr, nullptr);

		chat_field = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | ES_MULTILINE | WS_VSCROLL | WS_HSCROLL  | WS_BORDER | ES_READONLY, 0, 23, 518, 348, hwnd, nullptr, nullptr, nullptr);

		send_field = CreateWindow("EDIT", "", WS_VISIBLE | WS_CHILD | WS_BORDER | WS_HSCROLL | ES_MULTILINE, 0, 372, 450, 40, hwnd, (HMENU)2, nullptr, nullptr);
	
		send_button = CreateWindow("BUTTON", "SEND", WS_VISIBLE | WS_CHILD, 452, 373, 50, 20, hwnd, (HMENU)1, nullptr, nullptr);
		
		SetFocus(send_field);
	}
					break;
	case WM_DESTROY:
		closesocket(out);
		closesocket(in);
		WSACleanup();
		quick_exit(0); 
		break;
	default:
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

int WinMain(_In_ HINSTANCE hInstance, _In_opt_  HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow) { //Entry Point

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	UNREFERENCED_PARAMETER(nCmdShow);

	WNDCLASS window; //Create window class

	window.cbClsExtra = NULL;
	window.cbWndExtra = NULL;
	window.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	window.hCursor = LoadCursor(NULL, IDC_ARROW);
	window.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	window.hInstance = hInstance;
	window.lpfnWndProc = WndProc;
	window.lpszClassName = "just_window";
	window.lpszMenuName = nullptr;
	window.style = CS_HREDRAW | CS_VREDRAW;;

	RegisterClass(&window); //Register the defined window class

	CreateWindow("just_window", "Chat", WS_VISIBLE | WS_SYSMENU | WS_SIZEBOX | WS_MAXIMIZEBOX | WS_MINIMIZEBOX, 0, 0, 518, 435, nullptr, nullptr, hInstance, nullptr); //Create the defined window

	int wsOk = WSAStartup(version, &data);
	if (wsOk != 0)
	{
		// Not ok! Get out quickly
		display_msg("STARTING WINSOCK FAILED!");
	}
	
	server.sin_family = AF_INET; // AF_INET = IPv4 addresses
	server.sin_port = htons(PORT); // Little to big endian conversion
	inet_pton(AF_INET, IP.c_str(), &server.sin_addr); // Convert from string to byte array

	out = socket(AF_INET, SOCK_DGRAM, 0);

	std::thread my_slave(start_server);

	MSG msg; //Create message class

	while (GetMessage(&msg, NULL, 0, 0)) { //Message loop, get message
		TranslateMessage(&msg); //Read Message
		DispatchMessage(&msg); //Send message to WndProc
	}

	return (int)msg.wParam;
}