#include <vector>
#include <semaphore.h>
using namespace std;
typedef long long ll;

struct queen
{
    ll row, col, a, b;
    queen() {}
    queen(int row, int col, int a, int b) : row(row), col(col), a(a), b(b) {}
};

struct qbuf_t
{
    vector<queen> buf;
    int front;
    int rear;
    sem_t mutex;
    sem_t slots;
    sem_t items;
    qbuf_t(int n)
    {
        buf.resize(n);
        front = rear = 0;
        sem_init(&mutex, 0, 1);
        sem_init(&slots, 0, n);
        sem_init(&items, 0, 0);
    }

    void insert(queen q)
    {
        sem_wait(&slots);
        sem_wait(&mutex);
        buf[(++rear) % buf.size()] = q;
        sem_post(&mutex);
        sem_post(&items);
    }

    queen remove()
    {
        queen q;
        sem_wait(&items);
        sem_wait(&mutex);
        q = buf[(++front) % buf.size()];
        sem_post(&mutex);
        sem_post(&slots);
        return q;
    }
};

struct workgroub
{
    sem_t empty;
    sem_t mutex;
    int size = 0;
    ll ans = 0;
    workgroub()
    {
        sem_init(&empty, 0, 1);
        sem_init(&mutex, 0, 1);
    }

    void add()
    {
        sem_wait(&mutex);
        if (size == 0)
        {
            sem_wait(&empty);
        }
        size++;
        sem_post(&mutex);
    }

    void del(ll cnt)
    {
        sem_wait(&mutex);
        ans += cnt;
        size--;
        if (size == 0)
            sem_post(&empty);
        sem_post(&mutex);
    }

    void wait()
    {
        sem_wait(&empty);
        sem_post(&empty);
    }
};
