#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <string.h>
#include <time.h>
#include <sys/epoll.h>
#include <pthread.h>
#include "qbuf.h"
#include <set>
using namespace std;
typedef struct sockaddr SA;
typedef long long ll;
const int Size = 1024;
const int maxn = 128;

int listenfd();
void *allocwork(void *arg);
void *waitall(void *arg);
void *connectclnt(void *arg);
void prepare(ll row, ll col, ll a, ll b);

ll n = 16, sum, port;
workgroub wg;
qbuf_t qbuf = qbuf_t(Size);
sem_t sem;
set<int> st;
int main(int argc, const char **argv)
{
    if (argc != 2)
    {
        printf("Usage:%s <port>\n", argv[0]);
        exit(1);
    }
    sem_init(&sem, 0, 1);
    port = atoi(argv[1]);
    pthread_t pid;
    pthread_create(&pid, NULL, connectclnt, NULL);
    pthread_detach(pid);
    while (~scanf("%lld", &n))
    {
        if (n == 1)
        {
            printf("%lld queens problem result: 1\n", n);
            continue;
        }
        time_t start = time(NULL);
        sum = (n - 2) * (n - 1);
        prepare(0, 0, 0, 0);
        wg.wait();
        time_t end = time(NULL);
        int ti = end - start;
        printf("%lld queens problem result: %lld\ntakes %d s\n", n, wg.ans, ti);
        wg.ans = 0;
    }
    for (auto fd : st)
        close(fd);
    return 0;
}

void *connectclnt(void *arg)
{
    int serv_sock = listenfd(), clnt_sock;
    sem_wait(&sem);
    st.insert(serv_sock);
    sem.post(&sem);
    pthread_t pid;
    while (1)
    {
        struct sockaddr_in clnt_addr;
        socklen_t l = sizeof(clnt_addr);
        clnt_sock = accept(serv_sock, (SA *)&clnt_addr, &l);
        sem_wait(&sem);
        st.insert(clnt_sock);
        sem.post(&sem);
        printf("%s connected...\n", inet_ntoa(clnt_addr.sin_addr));
        int *cfd = (int *)alloca(sizeof(int));
        *cfd = clnt_sock;
        pthread_create(&pid, NULL, allocwork, (void *)cfd);
        pthread_detach(pid);
    }
    return NULL;
}

int listenfd()
{
    int fd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);
    socklen_t len = sizeof(serv_addr);
    bind(fd, (SA *)&serv_addr, len);
    listen(fd, 10);
    int opt = 1;
    len = sizeof(opt);
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (void *)&opt, len);

    return fd;
}

void *allocwork(void *arg)
{
    int fd = *((int *)arg), len;
    char buf[maxn];
    queen q = qbuf.remove();
    sprintf(buf, "%lld %lld %lld %lld %lld\n", n, q.row, q.col, q.a, q.b);
    write(fd, buf, sizeof(buf));
    while (len = read(fd, buf, maxn))
    {
        ll ans = atol(buf);
        wg.del(ans);
        printf("Completed %.2f%%  %lld\n", (1 - wg.size * 1.0 / sum) * 100, wg.ans);
        q = qbuf.remove();
        sprintf(buf, "%lld %lld %lld %lld %lld\n", n, q.row, q.col, q.a, q.b);
        write(fd, buf, sizeof(buf));
    }
    qbuf.insert(q);
    wg.add();
    close(fd);
    sem_wait(&sem);
    st.erase(serv_sock);
    sem.post(&sem);
    return NULL;
}

void prepare(ll row, ll col, ll a, ll b)
{
    ll available = ((1 << n) - 1) & ~(col | a | b);
    while (available)
    {
        ll p = available & -available;
        available ^= p;
        if (row == 1)
        {
            qbuf.insert(queen(row + 1, col | p, (a | p) >> 1, (b | p) << 1));
            wg.add();
        }
        else
            prepare(row + 1, col | p, (a | p) >> 1, (b | p) << 1);
    }
}
