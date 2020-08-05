#include <bits/stdc++.h>
#include "qbuf.h"
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/epoll.h>
using namespace std;
typedef long long ll;
const int Size = 100;
ll n;

void dfs(ll row, ll col, ll a, ll b, ll &cnt);
void *thread1(void *vargp);
workgroub wg;
qbuf_t qbuf = qbuf_t(Size);
char buf[128];

int main(int argc, const char **argv)
{
    if (argc != 3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }
    unsigned int ncpu = sysconf(_SC_NPROCESSORS_ONLN);
    pthread_t tid;
    for (int i = 0; i < ncpu; i++)
        pthread_create(&tid, NULL, thread1, NULL);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof serv_addr);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);
    serv_addr.sin_port = htons(atoi(argv[2]));
    int server_sock;
    server_sock = socket(PF_INET, SOCK_STREAM, 0);
    int err = connect(server_sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (err < 0)
    {
        fputs("can not connect", stdout);
        exit(2);
    }
    puts("connected");
    while (int len = read(server_sock, buf, 128))
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
        write(server_sock, buf, sizeof(buf));
        time_t end = time(NULL);
        int ti = end - start;
        printf("%lld queens problem is being calculated,current sub-problem time is %d s\n", n, ti);
        wg.ans = 0;
    }
    close(server_sock);
    return 0;
}

void *thread1(void *vargp)
{
    pthread_detach(pthread_self());
    while (1)
    {
        queen q = qbuf.remove();
        ll cnt = 0;
        dfs(q.row, q.col, q.a, q.b, cnt);
        wg.del(cnt);
    }
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
