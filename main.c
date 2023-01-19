#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<conio.h>
#include<stdbool.h>
#include<time.h>
#include<windows.h>
#include "helper_windows.h"
#include "colorize.h"

struct user{ // user info
    char username[64], password[64]; // max length of user and pass is 64
    char dif[3]; int score[3]; char time[3][255]; // difficulty, score, last time played
}player, vlayer;

int SCORE = 0, N = 0, V; // N is number of linked list nodes and V will be the save slot number after starting the game

void my_callback_on_key_arrival(char c); // this func being called every time a key is stroked after start listeting
void login(); // signs an existing account or adds a new one
bool user_exists(char *username); // checks if there exists any user with username in file and sets it to player if so
void new_game(); // player chooses one of his 3 save slots; then chooses if he wants to overwrite the save or continue it ( in which this save's score will be increased )

int main(){
    
    time_t T = time(NULL);
    srand((unsigned) time(&T));

    login();
    new_game();

    HANDLE thread_id = start_listening(my_callback_on_key_arrival); // starting to check for keystrokes


    player.score[V] += SCORE; // update score
    struct tm tm = *localtime(&T);
    sprintf(player.time[V],"%d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec); // update last time played
    // update data.txt
    FILE* f1 = fopen("data.txt","r");
    FILE* f2 = fopen("tmp.txt","w");
    fwrite(&player, sizeof(struct user), 1,f2);
    while(fread(&vlayer, sizeof(struct user), 1, f1)) if(vlayer.username != player.username) fwrite(&vlayer, sizeof(struct user), 1, f2);
    fclose(f1); fclose(f2);
    remove("data.txt"); rename("tmp.txt", "data.txt");

    WaitForSingleObject(thread_id,INFINITE);
    return 0;
}

void my_callback_on_key_arrival(char c){
}

bool user_exists(char* username){
    FILE* fpt = fopen("data.txt","r");
    while(fread(&vlayer, sizeof(struct user), 1, fpt)) if( !strcmp(vlayer.username, username) ){ player = vlayer; fclose(fpt); return 1; }
    fclose(fpt);
    return 0;
}

void login(){
    // innit player contents
    for(int i=0;i<3;i++) player.dif[i] = 0, player.score[i] = 0, strcpy(player.time[i],"");

    system("cls");
    printf("1.Sign in\n2.Register\nChoose an option: "); scanf("%d", &V);

    system("cls");
    while(1){
        printf("Username: "); scanf("%s", player.username);
        bool bb = user_exists(player.username);
        if(!bb){
            if(V==1){ // siging in
                printf("No player with that username\nWanna register?(y/n)"); scanf(" %c",&V);
                V = ( V=='y' ? 2:1 );
            }
            else{ // registering
                printf("Password: "); scanf("%s", player.password);
                break;
            }
        }
        else{
            if(V==1){ 
                do {
                    printf("Password: "); scanf("%s", vlayer.password);
                } while ( strcmp(vlayer.password, player.password) );
                break;
            }
            else{
                printf("username exists, please enter a new on\n");
            }
        }
    }
}

void new_game(){
    system("cls");
    for(int i=0;i<3;i++){
        printf("%d. ",i+1);
        if(!player.dif[i]) printf("empty slot\n");
        else printf("difficulty: %d, score: %d, last played: %s\n", player.dif[i], player.score[i], player.time[i]);
    }
    printf("Choose a slot to save: "); scanf("%d", &V); V--;
    
    if(!player.dif[V]) return;
    printf("Wanna overwrite or continue?(o/c) "); char c; scanf(" %c", &c);
    if(c=='o'){
        printf("1.Easy\n2.Medieum\n3.Hard\nChoose difficulty: "); scanf(" %d",&player.dif[V]);
        player.score[V] = 0;
    }
}