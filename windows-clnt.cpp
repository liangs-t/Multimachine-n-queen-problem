#include "qbuf.h"
#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <process.h>
#include <time.h>
#include <windows.h>
using namespace std;
typedef long long ll;
const int maxn = 256;
const int Size = 100;
ll n;

void dfs(ll row, ll col, ll a, ll b, ll &cnt);
unsigned WINAPI thread1(void *arg);
workgroub wg;
qbuf_t qbuf = qbuf_t(Size);
char buf[128];

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage:%s <IP> <port>\n", argv[0]);
        exit(1);
    }
    WSADATA wsadata;
    SOCKET ServSock, ClntSock;
    SOCKADDR_IN ServAddr, ClntAddr;
    int sz;
    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0)
        printf("startup error\n");
    ServSock = socket(PF_INET, SOCK_STREAM, 0);
    if (ServSock == INVALID_SOCKET)
        printf("socket error\n");
    memset(&ServAddr, 0, sizeof(ServAddr));
    ServAddr.sin_family = AF_INET;
    ServAddr.sin_addr.s_addr = inet_addr(argv[1]);
    ServAddr.sin_port = htons(atoi(argv[2]));
    if (connect(ServSock, (SOCKADDR *)&ServAddr, sizeof(ServAddr)) == SOCKET_ERROR)
    {
        printf("connect error\n");
        return 0;
    }
    puts("connected");
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    for (int i = 0; i < sysinfo.dwNumberOfProcessors; i++)
        _beginthreadex(NULL, 0, thread1, NULL, 0, NULL);
    while (int len = recv(ServSock, buf, 128, 0))
    {
        time_t start = time(NULL);
        queen q;
        sscanf(buf, "%lld %lld %lld %lld %lld", &n, &q.row, &q.col, &q.a, &q.b);
        ll available = ((1 << n) - 1) & ~(q.col | q.a | q.b);
        while (available)
        {
            ll r = available & -available;
            available ^= r;
            qbuf.insert(queen(q.row + 1, q.col | r, (q.a | r) >> 1, (q.b | r) << 1));
            wg.add();
        }
        wg.wait();
        sprintf(buf, "%lld", wg.ans);
        send(ServSock, buf, sizeof(buf), 0);
        time_t end = time(NULL);
        int ti = end - start;
        printf("%lld queens problem is being calculated,current sub-problem time is %d s\n", n, ti);
        wg.ans = 0;
    }
    closesocket(ServSock);
    return 0;
}

unsigned WINAPI thread1(void *arg)
{
    while (1)
    {
        queen q = qbuf.remove();
        ll cnt = 0;
        dfs(q.row, q.col, q.a, q.b, cnt);
        wg.del(cnt);
    }
    return 0;
}

void dfs(ll row, ll col, ll a, ll b, ll &cnt)
{
    ll available = ((1 << n) - 1) & ~(col | a | b);
    while (available)
    {
        ll p = available & -available;
        available ^= p;
        if (row == n - 1)
            cnt++;
        else
            dfs(row + 1, col | p, (a | p) >> 1, (b | p) << 1, cnt);
    }
}