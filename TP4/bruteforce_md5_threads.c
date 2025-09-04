// ldd: -pthread -lcrypto
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <openssl/md5.h>
#include <pthread.h>

#define NB_THREADS 10
#define NB_NONCES_MAX 5

int tableau_nonces[NB_NONCES_MAX];
int compteur_nonces = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
char hash[1 + 2 * MD5_DIGEST_LENGTH] = {0};

char *md5hash(char *str) {
    unsigned char md5[MD5_DIGEST_LENGTH] = {0};
    MD5(str, strlen(str), md5);
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++){
        sprintf(hash + 2 * i, "%02x", md5[i]);
    }
    return hash;
}


int zeros(char *s, int n) {
    int i = 0;
    while (s[i] != '\0' && s[i] == '0')
        i++;
    return i >= n;
}


void *bruteforce(void *arg) {
    int id = *(int *)arg;
    int i = id;
    int n, size;

    while (1) {
        pthread_mutex_lock(&mutex);
        if (compteur_nonces >= NB_NONCES_MAX) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        pthread_mutex_unlock(&mutex);

        n = i;
        size = 1;
        for (; n > 0; n /= 10) size++;
        char *s = malloc(size);
        s[size - 1] = '\0';
        n = i;
        int j = size - 2;
        while (j >= 0) {
            s[j] = '0' + (n % 10);
            n /= 10;
            j--;
        }

        char *hash = md5hash(s);
        if (zeros(hash, 6)) {
            pthread_mutex_lock(&mutex);
            if (compteur_nonces < NB_NONCES_MAX) {
                tableau_nonces[compteur_nonces] = i;
                printf(" le Thread  num %d a  reussi de trouver la nonce %d\n", id, i);
                compteur_nonces++;
            }
            pthread_mutex_unlock(&mutex);
            break;
        }
        free(s);
        i += NB_THREADS;
    }
    return NULL;
}


int main() {
    pthread_t threads[NB_THREADS];
    int ids[NB_THREADS];

    for (int i = 0; i < NB_THREADS; i++) {
        ids[i] = i;
        pthread_create(&threads[i], NULL, bruteforce, &ids[i]);
    }

    for (int i = 0; i < NB_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("\nNonces trouvées :\n");
    for (int i = 0; i < compteur_nonces; i++) {
        printf("%d\n", tableau_nonces[i]);
    }

    return 0;
}
