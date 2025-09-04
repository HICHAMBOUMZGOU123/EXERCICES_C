//ldd:-lcrypto
#include <openssl/md5.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int zeros (char *s, int n){
    int i;
    for (i=0;s[i] != '\0' && s[i] == '0'; i++);
    return(i>=n)? 1:0;
}

 
 
  
  char hash[1+ 2*MD5_DIGEST_LENGTH]={0};
 
  char* md5hash (char*str){
  unsigned char md5[MD5_DIGEST_LENGTH]={0};
  MD5(str,strlen(str), md5);
  for(int i=0; i<MD5_DIGEST_LENGTH;i++){
  sprintf(hash+ 2*i,"%02x",md5[i]);
  }
  return hash;
  }
void bruteforce (int first, int step, int zero){
    int i=first;
    int n;
    int size;
   
    while(1){
        n=i;
        size=1;
     for (; n > 0; n /= 10) {
           size++;
     }
      char *s=(char*)malloc(size*sizeof(char)); //j pas utiliser un tableau statique parce que size il est pas fixe 
       s[size-1]='\0';
        
        n=i;
       int j = size - 2;
     while (j >= 0) {
    s[j] = '0' + (n % 10);
    n =n/10;
    j--;
      } 
        char* hash= md5hash(s);
        if(zeros(hash, zero)==1){
            char nomF[20];
            sprintf(nomF,"found.%d",getpid());
            FILE* f=fopen(nomF,"w");
            fprintf(f,"%d",i);
            return;
        }
        
        free(s);
        i=i+step;
    }
}

int main() {
    int nbEnfants = 10;
    pid_t tab[10];
    int i = 0;

    while (i < nbEnfants) {
        pid_t pid = fork();  

        if (pid == 0) { 
            bruteforce(i, 10, 6);
            exit(0); 
        } else if (pid > 0) {  
            tab[i] = pid;
        } 
        i++;
    }
    return 0;
}
