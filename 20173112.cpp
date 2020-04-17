#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#pragma comment(lib, "ws2_32")

#include <iostream>
#include <WinSock2.h>
#include <signal.h>
#include <vector>



using namespace std;


const int LENGTH_NAME = 31;
const int LENGTH_MSG = 101;
const int LENGTH_SEND = 201;
const int N = 10;




typedef struct {
	SOCKET data;
	char ip[16];
	char name[LENGTH_NAME];
}node;

// Khởi tạo 1 node phần tử
node createNode(SOCKET s, char ip[], const char name[] = "NULL") {
	node p;
	p.data = s;
	strcpy(p.ip, ip);
	strcpy(p.name, name);
	return p;
}

node a[N];
bool disable[N] = { false };

int num = 1;


// Hàm chuyển tiếp dữ liệu đến client khác

void send_all_client(node p, char buff[]) {
	for (int i = 0; i < num; i++) {
		if (!disable[i] && p.data != a[i].data) {
			send(a[i].data, buff, strlen(buff), 0);
		}
	}
}



DWORD WINAPI ClientHander(LPVOID param) {

	bool leave = false;
	int i = (int)param -1; 
	node p = a[i];
	char nickname[LENGTH_NAME] = {};
	char recv_buff[LENGTH_MSG] = {};
	char send_buff[LENGTH_SEND] = {};

	int r = recv(p.data, nickname, LENGTH_NAME , 0);
	nickname[r - 1] = '\0';
	strcpy(p.name, nickname);



	cout << p.name << " ";
	cout << p.ip << " ";

	// thông báo 1 client đăng nhập

	cout << p.data << " join the chatroom" << endl;
	cout << send_buff << " join the chatroom" << endl;



	send_all_client(p, send_buff);

	while (!leave) {

		int r = recv(p.data, recv_buff, LENGTH_MSG, 0);

		if (strlen(recv_buff) == 0) {
			continue;
		}
		if (r > 0) {
			recv_buff[r - 1] = '\0';

			if (strcmp(recv_buff, "quit") != 0) {
				cout << send_buff << " " << p.name << " " << recv_buff << endl;				
			}
			else{
				printf("%s(%s)(%d) leave the chatroom\n", p.name, p.ip, p.data);
				sprintf(send_buff, "%s:%s leaves the chatroom\n", p.name, p.ip);
				leave = true;
			}

			send_all_client(p, send_buff);




		}
		

	}
	closesocket(p.data);
	disable[i] = true;
	return 0;

}

// hàm thoát đăng nhập 
void ctrl_c(int signum) {
	for (int i = 0; i < num; i++) {
		if (!disable[i]) {
			cout << "Close socket";
			cout << a[i].data << endl;
			closesocket(a[i].data);
		}
	}
	cout << "log out" << endl;
}

int main() {


	signal(SIGINT, ctrl_c);

	WSADATA data;
	if (WSAStartup(MAKEWORD(2, 2), &data) == 0) {
		SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		sockaddr_in saddr, caddr;

		int clen = sizeof(caddr);
		int slen = sizeof(saddr);
		saddr.sin_family = AF_INET;
		saddr.sin_port = htons(8888);
		saddr.sin_addr.s_addr = htonl(INADDR_ANY);


		bind(s, (SOCKADDR*)&saddr, slen);

		getsockname(s, (SOCKADDR*)&saddr, &slen);
		cout << "Server start on " << inet_ntoa(saddr.sin_addr) << ntohs(saddr.sin_port) << endl;


		listen(s, 10);


		
		a[0] = createNode(s, inet_ntoa(saddr.sin_addr));

		while (true) {
			SOCKET c = accept(s, (SOCKADDR*)&caddr, &clen);

			getpeername(c, (SOCKADDR*)&caddr, &clen);
			cout << "Client " << inet_ntoa(caddr.sin_addr) << ntohs(caddr.sin_port) << "come" << endl;

			a[num++] = createNode(c, inet_ntoa(caddr.sin_addr));
			CreateThread(NULL, 0, ClientHander, (LPVOID)&num, 0, NULL);

		}

	}
}