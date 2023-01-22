// Please read github README first
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <conio.h>
#include <time.h>
#include <windows.h>
#include <unistd.h>
#include <memory.h>
#include "helper_windows.h"
#include "colorize.h"
#include "sha256.c"

/* We concatenate SALT and PEPPER (at the end) of the text we wanna hash so that we make it harder to get leaked.
    Salt is a random generated 4-char-length User specific string (so that Users with same password may have different hashes)
    Pepper is just a const string
    Also we hash it ITER times to make it take much longer for any alogrithm to find the original string */
const char* PEPPER = "CLASS";
const int   ITER   = 205;

struct User{ // User info
    char username[32], SALT[5]; BYTE password[SHA256_BLOCK_SIZE]; // max length of username is 32 and password is a 32 byte hash digest
    char dif[3]; int score[3], winstreak[3]; char time[3][255];   // difficulty, score, winstreak, last time played
}player, vlayer; // player is the currently playing User and vlayer is an assisstant variable (to use in files and stuff)

struct Node{
    char word[24];
    int type_cur;     // till which char is typed by the player
    bool yellows[24]; // if 1 means typed wrong
    int yellow_num;   // number of characters typed wrong
    bool isvague;
    struct Node* next;
    struct Node* prev;
}Head, Tail;

void  login();                                       // signs in an existing account or adds a new one
bool  User_exists(char *username);                   // checks if there exists any User with the given username in file and sets it to player if so
void  new_game();                                    // player chooses one of his 3 save slots; then chooses if he wants to overwrite the save or continue it ( in which this save's score will be increased )
char* rwg(int l, int r, bool ishard);                // rwg stands for random word generator and generates a random word of size [l,r)
BYTE* hash(char* s, char* SALT);                     // using sha256 hash function
void  my_callback_on_key_arrival(unsigned char c);   // this func being called every time a key is stroked after start listening
void  draw_board();                                  // draws the game table
int   get_y(int size);                               // gives a suggested y to print the word with the given size so that it looks almost in the middle of table
char* difname(int x);                                // returns the string name of difficulty based on player.dif integer

int SCORE = 0, V;          // V will be the save slot number after starting the game
int N = 0, W = 1;          // N is number of linked list Nodes and W is number of wave
int TCORRECT = 0;          // number of correct   inputed letters
int TWRONG   = 0;          // number of incorrect inputed letters

double       Y;            // Current wave's time
const double EY=0.000001;  // sleep precision
const double DY[] ={
        0.8, 0.7, 0.6   }; // decrement rate of wave's time for each difficulty
short int I = 1;           // hardness of words given in the current wave
bool ISVAGUE = 0;          // to check if we could give a vague word
int  HAND  = 0;            // HAND to play one handed { 0 for both, 1 for left, 2 for right }
bool LCASE = 0;            // LCASE: letter case { 0 lowercase, 1 both lower and upper }

const int WIDTH = 30, HEIGHT = 20;
struct Node* CUR = &Head;  // CUR is the currently typing Node
int CUR_LINE = -1;         // cursor line in the game

FILE* WORDS_FILE[3];       // for which { 0 are easy words, 1 long words, 2 hard words }
char KEYS[]     = "12345qwertasdfgzxcvb67890yuiophjkl;nm,.?" ;
char UNICASE[]  = "1234567890;,.?";
char HARDKEYS[] = "!@$%%*_-+";

// linked list functions

struct Node* create_Node(char* s){
    struct Node* new_Node = malloc(sizeof(struct Node));
    strcpy(new_Node->word, s);
    new_Node->type_cur = new_Node->yellow_num = 0;
    memset(new_Node->yellows, 0, 24*sizeof(bool));
    new_Node->isvague = ( ISVAGUE ? rand()%2 : 0 );
    new_Node->prev = new_Node->next = NULL;
    return new_Node;
}

struct Node* get_Node(int index){
    struct Node* cur_Node = Head.next;
    while(index--) cur_Node = cur_Node->next;
    return cur_Node;
}

void push_front(char* s){
    struct Node* new_Node = create_Node(s);
    new_Node->prev = &Head, new_Node->next = Head.next;
    (Head.next)->prev = new_Node;
    Head.next = new_Node;
    N++;
}

void delete_Node(struct Node* cur_Node){
    (cur_Node->prev)->next = cur_Node->next;
    (cur_Node->next)->prev = cur_Node->prev;
    N--;
}

int main()
{
    // initialize time struct and random number generator
    time_t T=time(NULL);
    struct tm tm = *localtime(&T);
    srand((unsigned) time(&T));

    // innitialize player contents
    for(int i=0;i<3;i++) player.dif[i] = player.score[i] = player.winstreak[i] = 0, strcpy(player.time[i],"0");

    // input game settings
    login(); new_game();

    // initialize word files
    WORDS_FILE[0] = fopen("words0.txt","w"); for(int i=0;i<200;i++) fprintf(WORDS_FILE[0],"%s\n", rwg(1,11,0));  fclose(WORDS_FILE[0]);
    WORDS_FILE[1] = fopen("words1.txt","w"); for(int i=0;i<200;i++) fprintf(WORDS_FILE[1],"%s\n", rwg(10,21,0)); fclose(WORDS_FILE[1]);
    WORDS_FILE[2] = fopen("words2.txt","w"); for(int i=0;i<200;i++) fprintf(WORDS_FILE[2],"%s\n", rwg(1,21,1));  fclose(WORDS_FILE[2]);

    // initialize linked list
    Head = *create_Node(""), Tail = *create_Node("");
    Head.next = &Tail, Tail.prev = &Head;

    // start the game
    HANDLE thread_id = start_listening(my_callback_on_key_arrival);
    clock_t START_TIME = clock();

    WORDS_FILE[0] = fopen("words0.txt","r");
    WORDS_FILE[1] = fopen("words1.txt","r");
    WORDS_FILE[2] = fopen("words2.txt","r");
    char new_word[24];
    for(int ii=0;ii<7-player.dif[V];ii++){
        // set word difficulty
        switch(player.dif[V]){
            case(1):
                if(ii==4) I++, ISVAGUE = 1;
                break;
            case(2):
                if(ii>2) I++;
            case(3):
                ISVAGUE = 1;
                if(ii>0 && ii<3) I++;
                break;
        }
        // add new wave to linked list
        for(int i=0;i<10;i++){
            fscanf(WORDS_FILE[rand()%I], "%s", new_word);
            push_front(new_word); CUR_LINE++;
            if(CUR->prev == NULL) CUR = CUR->next;
            if(N>HEIGHT){ // Lost the game
                system("cls");
                printf("\t\t\tYou Lost\n %s mode wave %d\tscore: 0\nPlay time: %.2lf seconds\tPrecision: %d%%\n",
                        difname(player.dif[V]), W,
                            (double)(clock()-START_TIME)/CLOCKS_PER_SEC, (int)((double)TCORRECT/(TCORRECT+TWRONG)*100));
                player.dif[V] = N = 0; 
                goto UPDATE_PHASE;
            }
            draw_board();
            clock_t target = clock() + (Y/EY*EY) * CLOCKS_PER_SEC ;
            while( clock() < target && N );
            draw_board();
        } W++;
        // decrease add word interval
        Y*=DY[player.dif[V]-1];
    }
    while(N);
    system("cls");
    printf("\t\t\tYou've reached a considerable speed in typing\n%s mode wave %d\tscore: %d\ttotal score: %d\nfinished in: %.2lf seconds\tPrecision: %d%%\n",
            difname(player.dif[V]), W, SCORE, player.score[V] += SCORE,
                    (double)(clock()-START_TIME)/CLOCKS_PER_SEC, (int)((double)TCORRECT/(TCORRECT+TWRONG)*100));
    player.winstreak[V]++;
    UPDATE_PHASE:
    for(int i=0;i<3;i++) fclose(WORDS_FILE[i]);

    // update last time played
    sprintf(player.time[V], "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
    // update data.txt
    FILE* f1 = fopen("data.txt","r");
    FILE* f2 = fopen("tmp.txt","w");
    fwrite(&player, sizeof(struct User), 1,f2);
    while(fread(&vlayer, sizeof(struct User), 1, f1)) if( strcmp(vlayer.username, player.username) ) fwrite(&vlayer, sizeof(struct User), 1, f2);
    fclose(f1); fclose(f2);
    remove("data.txt"); rename("tmp.txt", "data.txt");

    printf("Thanks for playing  "); for(int i=5;i;i--){ printf("\b%d",i); sleep(1); }
    exit(0);
    WaitForSingleObject(thread_id,INFINITE);
    return 0;
}

char* difname(int x){
    return (x==1 ? "Easy":(x==2 ? "Medium":"Hard"));
}

int get_y(int size){
    return WIDTH/2 - (size+1)/2;
}

void draw_board(){
    system("cls");
    struct Node* cur_Node = &Head;
    for(int i=0,size;i<HEIGHT;i++){
        setcolor(2); printf("#"); if(i==CUR_LINE) printf("->");
        if(i<N){
            cur_Node = cur_Node->next;
            gotoxy(i, get_y(size=strlen(cur_Node->word)));
            for(int j=0;j<size;j++){
                if( cur_Node->isvague && i!=CUR_LINE ){ setcolor(15); printf("#"); continue; }
                if(cur_Node->yellows[j])          setcolor(6);
                else if( j < cur_Node->type_cur ) setcolor(4);
                else                              setcolor(15);
                printf("%c",cur_Node->word[j]);
            }
        }
        gotoxy(i, WIDTH);
        setcolor(2); printf("#"); if(i==CUR_LINE) printf("\b\b\b<-#");
        printf("\n");
    }
    for(int i=0;i<=WIDTH;i++) printf("#");
    setcolor(15);
    gotoxy(0, WIDTH+2); printf("%s", player.username);
    gotoxy(1, WIDTH+2); printf("score: %d", SCORE);
    gotoxy(2, WIDTH+2); printf("%s mode wave %d", difname(player.dif[V]), W);
    gotoxy(3, WIDTH+2); printf("Errors: %d", TWRONG);
}

void my_callback_on_key_arrival(unsigned char c){ static bool ISARROW = 0;
    if(!N) return;
    int size = strlen(CUR->word);
    if( c == 224 )    ISARROW = 1;
    else if(ISARROW){ ISARROW = 0;
        if     ( c == 72 ){ if( CUR_LINE != 0   ) CUR = CUR->prev, CUR_LINE--; }
        else if( c == 80 ){ if( CUR_LINE != N-1 ) CUR = CUR->next, CUR_LINE++; }
    }
    else if( c == '\b'){ if(CUR->type_cur) CUR->yellow_num -= CUR->yellows[--(CUR->type_cur)], CUR->yellows[CUR->type_cur] = 0; }
    else if( c == CUR->word[CUR->type_cur] ){ TCORRECT++; if( ++(CUR->type_cur) == size && !(CUR->yellow_num) ){
        delete_Node(CUR); CUR_LINE -= (CUR_LINE==N);
        CUR = ( (CUR->next)->next != NULL ? CUR->next : CUR->prev);
        SCORE+=1+LCASE+CUR->isvague+(size>10);for(int i=0;i<strlen(HARDKEYS);i++)if(strchr(CUR->word,HARDKEYS[i])!=NULL){SCORE++;break;}
    }}
    else if( CUR->type_cur < size ) TWRONG++, (CUR->yellow_num) += CUR->yellows[(CUR->type_cur)++] = 1;
    draw_board();
}

BYTE* hash(char* s, char* SALT){
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
        else{ // 1/4 chance for a hard letter
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

bool User_exists(char* username){
    FILE* fpt = fopen("data.txt","r");
    while(fread(&vlayer, sizeof(struct User), 1, fpt)) if( !strcmp(vlayer.username, username) ){ player = vlayer; fclose(fpt); return 1; }
    fclose(fpt);
    return 0;
}

void login(){ static char password[75];
    system("cls");
    printf("1.Sign in\n2.Register\nChoose an option: "); scanf("%d", &V);

    system("cls");
    while(1){
        printf("username: "); scanf("%s", player.username);
        bool bb = User_exists(player.username);
        if(!bb){ // User doesn't exist
            if(V==1){ // siging in
                printf("No player with that username\nWanna register?(y/n)"); scanf(" %c",&V);
                V = ( V=='y' ? 2:1 );
            }
            else{ // registering
                printf("password: "); scanf("%s", password);
                memcpy(player.password, hash(password, strcpy(player.SALT, rwg(4,5,1))), SHA256_BLOCK_SIZE);
                break;
            }
        }
        else{ // User exists
            if(V==1){  // siging in
                do {
                    printf("password: "); scanf("%s", password);
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
        else printf("difficulty: %s, score: %d, streak: %d, last played: %s\n", 
            	        difname(player.dif[i]), player.score[i], player.winstreak[i], player.time[i]);
    }
    printf("Choose a slot to save: "); scanf("%d", &V); V--;

    if(player.dif[V]){
        printf("Wanna overwrite or continue?(o/c) "); scanf(" %c", &c);
    }
    if(!player.dif[V] || c=='o'){
        printf("1.Easy\n2.Medieum\n3.Hard\nChoose difficulty: "); scanf(" %d", &player.dif[V]);
        player.score[V] =  player.winstreak[V] = 0;
    }
    switch(player.dif[V]){
        case(1): Y = 10; break;
        case(2): Y = 8; break;
        case(3): Y = 5; break;
    }

    printf("1.NO\n2.Only left hand\n3.Only right hand\n");
    printf("Wanna play one handed? "); scanf(" %c",&c);
    HAND = c-'0';
    printf("Wanna type both upper and lower case letters?(y/n)"); scanf(" %c",&c);
    LCASE = (c=='y' ? 1:0);
}