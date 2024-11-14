#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
using namespace std;


int main()
{
    // Try to use socket -- killed
    int socket_desc = socket(AF_INET, SOCK_STREAM, 0);

    if (-1 == socket_desc) {
        perror("cannot create socket");
        exit(1);
    }
}