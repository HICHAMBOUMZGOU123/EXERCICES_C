//ldd:-lcrypto

#include <openssl/md5.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <poll.h>
#include <signal.h>

int tube_sortie;

int zeros(char *s, int n)
{
    int i = 0;
    while (s[i] != '\0' && s[i] == '0')
        i++;
    if (i >= n)
        return 1;
    else
        return 0;
}


char hash[1 + 2 * MD5_DIGEST_LENGTH] = {0};

char *md5hash(char *str)
{
    unsigned char md5[MD5_DIGEST_LENGTH] = {0};
    MD5(str, strlen(str), md5);
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++)
    {
        sprintf(hash + 2 * i, "%02x", md5[i]);
    }
    return hash;
}

void bruteforce(int first, int step, int zero)
{
    int i = first;
    int n, size;

    while (1) {
        n = i;
        size = 1;
        for (; n > 0; n /= 10) size++;

        char *s = (char *)malloc(size * sizeof(char));
        s[size - 1] = '\0';

        n = i;
        int j = size - 2;
        while (j >= 0) {
            s[j] = '0' + (n % 10);
            n /= 10;
            j--;
        }

        char *hash = md5hash(s);
        if (zeros(hash, zero) == 1)
        {
            write(tube_sortie, &i, sizeof(i));
            return;
        }
        free(s);
        i = i + step;
    }
}

int main()
{
    int nbEnfants = 10;
    pid_t tab[10];
    int tube[20];
    for (int i = 0; i < nbEnfants; i++)   {
        pipe(&tube[2 * i]);
       tube_sortie = tube[2 * i + 1];
        tab[i] = fork();
        if (tab[i] == 0) {// seulement le fils execute un bruteforme 
           
            bruteforce(i, 10, 6);
            exit(0); 
           
        }
        close(tube[2 * i + 1]);
    }
    int donnees;
    int nb_nomces_recues = 0;
    while (nb_nomces_recues < 5) 
    {
    for (int i = 0; i < nbEnfants; i++) {
     if (tab[i] != -1) {
      struct pollfd tube_poll;
      tube_poll.fd = tube[2 * i];
       tube_poll.events = POLLIN;
        int status = poll(&tube_poll, 1, 10);
                if (status > 0 && (tube_poll.revents & POLLIN))
                {
         read(tube[2 * i], &donnees, sizeof(donnees));
           printf("Processus pid %d a envoyé la nonce %d\n", tab[i], donnees);
         nb_nomces_recues++;
         tab[i] = -1;
       }
       }
      }
    }
    for (int i = 0; i < nbEnfants; i++) 
    {
      if (tab[i]!=-1)
        kill(tab[i],SIGTERM);

    }

    return 0;
}
