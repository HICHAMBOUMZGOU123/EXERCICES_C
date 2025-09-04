//version 3
// ldd: -pthread -lrt
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <mqueue.h>
#include <semaphore.h>


int main (int argc, char *argv[]) {
  mq_unlink("/hicham");

    
    int data_1[8] = {5, 8, 3, 1, 7, 3, 7, 4};
    test_sort(data_1, sizeof(data_1)/sizeof(data_1[0]), "data 1");


   
    int len_2 = 20;
    int data_2[len_2];
    int i, j;

    j=len_2;
    for (i=0; i<len_2; i++)
        data_2[i] = j--;

    test_sort(data_2, len_2, "data 2");

    
    int len_3 = 10;
    int *data_3 = calloc(len_3, sizeof(int));

    j=len_3;
    for (i=0; i<len_3; i++)
        data_3[i] = j--;

    test_sort(data_3, len_3, "data 3");
    free(data_3);
                                             
    exit(0);
}


typedef unsigned int uint;

typedef struct
{
    uint len;
    int* val;
} Tab;



void test_sort (int * data, int len, const char *name)
{
    Tab t = { .len = len, .val = data };
    printf("%s before : ", name);
    print_tab(t);

    sort(t);

    printf("%s after  : ", name);
    print_tab(t);
    printf("\n");
}
typedef union   
{
    Tab tab;
    char raw[sizeof(Tab)];
} msg_t;

mqd_t queue;
sem_t sem;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
Tab slice (Tab t, uint first, uint last)
{
    Tab s;
    s.len = last - first + 1;
    s.val = t.val + first;
    return s;
}

uint partition (Tab t)
{
    int pivot = t.val[(t.len-1)/2];
    uint a = -1;
    uint b = t.len;
    while (1)
    {
        do
        {
            a++;
        }
        while (t.val[a] < pivot);
        do
        {
            b--;
        }
        while (t.val[b] > pivot);
        if (a >= b)
        {
            return b;
        }
        swap(t, a, b);
    }
}



void* sort_thread (void *arg)
{

    msg_t msg;
    while (1)
    {

        mq_receive(queue, msg.raw, sizeof(msg_t), NULL);
        Tab t = msg.tab;

        
        uint p;

        p = partition(t);

        Tab t_left = slice(t, 0, p);
        if (t_left.len>1) {
            
            msg.tab = t_left;
            mq_send(queue, msg.raw, sizeof(msg_t), 0);
        }
        else
            sem_wait(&sem);

        Tab t_right = slice(t, p+1, t.len-1);
        if (t_right.len>1) {
        
            msg.tab = t_right;
            mq_send(queue, msg.raw, sizeof(msg_t), 0);
        }
        else
            sem_wait(&sem);

        int sval;
        sem_getvalue(&sem, &sval);
        if (sval  == 0)
            pthread_cond_signal(&cond);

    }
}

void sort (Tab tab)
{
    int nb_threads = 10; 
    sem_init(&sem, 0, tab.len);

    int i;
    msg_t msg;

    pthread_t threads[nb_threads]; 
    int targs[nb_threads]; 

    
    struct mq_attr attr = { .mq_maxmsg = tab.len/2,
                            .mq_msgsize = sizeof(msg_t)
    };
  mq_unlink("/hicham");
    queue = mq_open("/hicham", O_RDWR|O_CREAT|O_EXCL, 0660, &attr);

    
    if(queue == (mqd_t) -1) {
        perror("Error in mq_open");
        printf("Remark : number of ints to sort must be <= 2*msg_max\n");
        printf("default msg_max = 10\n\n");
        printf("See /proc/sys/fs/mqueue/msg_max\n");
        printf("https://www.man7.org/linux/man-pages/man7/mq_overview.7.html\n");
        exit(-1);
    }

    for (i=0; i<nb_threads; i++)   
    {
        targs[i] = i;
        pthread_create(threads+i, NULL, sort_thread, targs+i);
    }

    msg.tab = tab;
    mq_send(queue, msg.raw, sizeof(msg_t), 0);

    
    pthread_mutex_lock(&lock);
    pthread_cond_wait(&cond, &lock);
    pthread_mutex_unlock(&lock);

    for (i=0; i<nb_threads; i++)   
    {
        pthread_cancel(threads[i]); 
        pthread_join(threads[i], NULL);
    }
    mq_close(queue); 
    mq_unlink("/hicham"); 
    sem_destroy(&sem);
}

void swap (Tab t, uint i, uint j)
{
    int tmp = t.val[i];
    t.val[i] = t.val[j];
    t.val[j] = tmp;
}


void print_tab(Tab t)
{
    int i;

    for (i=0; i<t.len; i++)
        printf("%i ", t.val[i]);
    printf("\n");
}
