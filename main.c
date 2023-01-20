#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <conio.h>
#include <time.h>
#include <windows.h>
#include <memory.h>
#include "helper_windows.h"
#include "colorize.h"
#include "sha256.c"

/* We concatenate SALT and PEPPER at the end of the text we wanna hash so that we make it harder to get leaked.
    Salt is a random generated 4-char-length user specific string (so that users with same password may have different hashes)
    Pepper is a const string
    Also we hash it ITER times to make it take much longer for any alogrithm to find the original string */
const char* PEPPER = "CLASS";
const int ITER = 205;

struct user{ // user info
    char username[32], SALT[5]; BYTE password[SHA256_BLOCK_SIZE]; // max length of username is 32 and password is a 32 byte hash digest
    char dif[3]; int score[3]; char time[3][255];                 // difficulty, score, last time played
}player, vlayer; // player is the currently playing user and vlayer is an assisstant variable (to use in files and stuff)

void my_callback_on_key_arrival(char c);   // this func being called every time a key is stroked after start listening
void login();                              // signs in an existing account or adds a new one
bool user_exists(char *username);          // checks if there exists any user with the given username in file and sets it to player if so
void new_game();                           // player chooses one of his 3 save slots; then chooses if he wants to overwrite the save or continue it ( in which this save's score will be increased )
char* rwg(int l, int r, bool ishard);      // rwg stands for random word generator and generates a random of size [l,r)

int SCORE = 0, N = 0, V; // N is number of linked list nodes and V will be the save slot number after starting the game
float Y;                 // Current wave's time
int I = 0;               // hardness of words given in the current wave
int HAND = 0;            // HAND to play one handed { 0 for both, 1 for left, 2 for right }
bool LCASE = 1;          // LCASE: letter case { 0 lowercase, 1 both lower and upper }
FILE* WORDS_FILE[3];     // for which { 0 are easy words, 1 long words, 2 hard words }
char KEYS[] = "12345qwertasdfgzxcvb67890yuiophjkl;nm,./" ;
char UNICASE[] = "1234567890;,./";
char HARDKEYS[] = "!@$%%*?-+";

BYTE* hash(char* s, char* SALT){ // using sha256 hash function
    strcat(s, SALT); strcat(s, PEPPER);
    int size = strlen(s) + 1;
    BYTE* text = malloc( size * sizeof(BYTE) ); memcpy(text, s, size);
    BYTE* buff = malloc( SHA256_BLOCK_SIZE * sizeof(BYTE) );
    SHA256_CTX ctx;
    sha256_init(&ctx);
    for(int i=0;i<ITER;i++) sha256_update(&ctx, text, strlen(text));
    sha256_final(&ctx, buff);
    return buff;
}

int main()
{
    time_t T=time(NULL);
    srand((unsigned) time(&T));

    //login(); new_game();

    // make word files
    WORDS_FILE[0] = fopen("words0.txt","w"); for(int i=0;i<100;i++) fprintf(WORDS_FILE[0],"%s\n", rwg(1,11,0)); fclose(WORDS_FILE[0]);
    WORDS_FILE[1] = fopen("words1.txt","w"); for(int i=0;i<100;i++) fprintf(WORDS_FILE[1],"%s\n", rwg(10,21,0)); fclose(WORDS_FILE[1]);
    WORDS_FILE[2] = fopen("words2.txt","w"); for(int i=0;i<100;i++) fprintf(WORDS_FILE[2],"%s\n", rwg(1,21,1)); fclose(WORDS_FILE[2]);
    return 0;

    HANDLE thread_id = start_listening(my_callback_on_key_arrival);

    // update score
    player.score[V] += SCORE;
    // update last time played
    struct tm tm = *localtime(&T);
    sprintf(player.time[V], "%d-%02d-%02d %02d:%02d:%02d"
                            , tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    // update data.txt
    FILE* f1 = fopen("data.txt","r");
    FILE* f2 = fopen("tmp.txt","w");
    fwrite(&player, sizeof(struct user), 1,f2);
    while(fread(&vlayer, sizeof(struct user), 1, f1)) if( strcmp(vlayer.username, player.username) ) fwrite(&vlayer, sizeof(struct user), 1, f2);
    fclose(f1); fclose(f2);
    remove("data.txt"); rename("tmp.txt", "data.txt");

    WaitForSingleObject(thread_id,INFINITE);
    return 0;
}

void my_callback_on_key_arrival(char c){
}

char* rwg(int l, int r, bool ishard){
    int size = l + rand() % (r-l);
    char* c = malloc(size+1); c[size] = '\0';
    bool hard_flag = 0;
    for(int i=0,r;i<size;i++){
        if(!ishard || rand()%4){
            r = rand()%20;
            if(HAND==1)    c[i] = KEYS[r];
            else if(!HAND) c[i] = KEYS[r + 20*(rand()%2)];
            else           c[i] = KEYS[r + 20];
            if(LCASE && strchr(UNICASE, c[i])==NULL) c[i] += ('A'-'a')*(rand()%2);
        }
        else{ // 1/4 chance for a vague key
            r = rand()%4;
            if(HAND==1)    c[i] = HARDKEYS[r];
            else if(!HAND) c[i] = HARDKEYS[r + 4*(rand()%2)];
            else           c[i] = HARDKEYS[r + 4];
            hard_flag = 1;
        }
    }
    if(ishard && !hard_flag) return rwg(l,r,ishard);
    return c;
}

bool user_exists(char* username){
    FILE* fpt = fopen("data.txt","r");
    while(fread(&vlayer, sizeof(struct user), 1, fpt)) if( !strcmp(vlayer.username, username) ){ player = vlayer; fclose(fpt); return 1; }
    fclose(fpt);
    return 0;
}

void login(){ static char password[75];
    // innitialize player contents
    for(int i=0;i<3;i++) player.dif[i] = player.score[i] = 0, strcpy(player.time[i],"0");

    system("cls");
    printf("1.Sign in\n2.Register\nChoose an option: "); scanf("%d", &V);

    system("cls");
    while(1){
        printf("Username: "); scanf("%s", player.username);
        bool bb = user_exists(player.username);
        if(!bb){ // user doesn't exist
            if(V==1){ // siging in
                printf("No player with that username\nWanna register?(y/n)"); scanf(" %c",&V);
                V = ( V=='y' ? 2:1 );
            }
            else{ // registering
                printf("Password: "); scanf("%s", password);
                memcpy(player.password, hash(password, strcpy(player.SALT, rwg(4,5,1))), SHA256_BLOCK_SIZE);
                break;
            }
        }
        else{ // user exists
            if(V==1){  // siging in
                do {
                    printf("Password: "); scanf("%s", password);
                } while ( memcmp(hash(password, player.SALT), player.password, SHA256_BLOCK_SIZE) );
                break;
            }
            else{ // registering
                printf("username exists, please enter a new one\n");
            }
        }
    }
}

void new_game(){ static char c='0';
    system("cls");
    for(int i=0;i<3;i++){
        printf("%d. ",i+1);
        if(!player.dif[i]) printf("empty slot\n");
        else printf("difficulty: %d, score: %d, last played: %s\n", player.dif[i], player.score[i], player.time[i]);
    }
    printf("Choose a slot to save: "); scanf("%d", &V); V--;
    
    if(player.dif[V]){
        printf("Wanna overwrite or continue?(o/c) "); scanf(" %c", &c);
    }
    if(!player.dif[V] || c=='o'){
        printf("1.Easy\n2.Medieum\n3.Hard\nChoose difficulty: "); scanf(" %d", &player.dif[V]);
        player.score[V] = 0;
    }
    printf("1.NO\n2.Only left hand\n3.Only right hand\n");
    printf("Wanna play one handed? "); scanf(" %c",&c);
    HAND = c-'0';
    printf("Wanna type both upper and lower case letters?(y/n)"); scanf(" %c",&c);
    LCASE = (c=='y' ? 1:0);
}