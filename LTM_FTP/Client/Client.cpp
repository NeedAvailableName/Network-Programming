#include "Resources.h"
#include "UI.h"


int main(int argc, char* argv[]) {
	struct Window* fwin =  reinterpret_cast<Window*>(malloc(sizeof(Window)));
	//Step 1: Inittiate WinSock
	WSADATA wsaData;
	WORD wVersion = MAKEWORD(2, 2);
	if (WSAStartup(wVersion, &wsaData))
		cout << "Version is not supported" << endl;

	//Step 2: Construct socket	
	// SOCKET client;
	client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	//Step 3: Specify server address
	char* server = (char*)malloc(sizeof(char) * 10);
	char* port = (char*)malloc(sizeof(char) * 10);
	strcpy(server, "127.0.0.1");
	cout << "Input port: ";
	cin >> port;
	cout << server << " " << port <<endl;

	//(optional) Set time-out for receiving
	// int tv = 10000; //Time-out interval: 10000ms
	// setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (const char*)(&tv), sizeof(int));
	
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons((unsigned short)atoi(port));
	serverAddr.sin_addr.s_addr = inet_addr(server);
	// serverAddr.sin_port = htons((unsigned short)atoi((char*)argv[2]));
	// serverAddr.sin_addr.s_addr = inet_addr(argv[1]);

	//Step 4: Request to connect server
	if (connect(client, (sockaddr*)&serverAddr, sizeof(serverAddr))) {
		cout << "Error! Cannot connect server. " << WSAGetLastError() << endl;
		return 0;
	}
	/*-------------------------------------------------------------------*/
	createFolderLocal(CLIENT_FOLDER);
	
	// Init gtk
	int status;
	fwin->app = gtk_application_new ("com.example.GtkApplication", G_APPLICATION_NON_UNIQUE);
	g_signal_connect (fwin->app , "activate", G_CALLBACK (activate), fwin);
	// g_signal_connect (fwin->app , "command-line", G_CALLBACK (command_line), fwin);
	// g_signal_connect (app, "open", G_CALLBACK (on_open), NULL);
	status = g_application_run (G_APPLICATION (fwin->app ), argc, argv);
	g_object_unref (fwin->app );
	printf("Status %d\n", status);

	//Step 5: Communicate with server
	//MENU
	

	
	// int scene = 1;
	// // Account acc;
	// // Group gr;
	// do {
	// 	if (scene == 1) {
	// 		scene = optionFirstFrame(client, acc, argc, argv);
	// 		system("pause");
	// 	}
	// 	if (scene == 2) {
	// 		scene = optionSecondFrame(client, acc, gr);
	// 		system("pause");
	// 	}
	// 	if (scene == 3) {
	// 		scene = optionThirdFrame(client, acc, gr);
	// 		system("pause");
	// 	}
	// } while (scene != 4);
	/*-------------------------------------------------------------*/
	printf("Closed connection.\n");
	logout(client, acc.id);
	//Step 6: Close socket
	closesocket(client);
	//Step 7: Terminate Winsock
	WSACleanup();
	return 0;
}
