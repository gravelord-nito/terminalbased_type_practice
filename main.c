#include<stdio.h>
#include<stdlib.h>
#include "helper_windows.h"
#include "colorize.h"

void my_callback_on_key_arrival(char c);

int main(){
    HANDLE thread_id = start_listening(my_callback_on_key_arrival);
    
    WaitForSingleObject(thread_id,INFINITE);
    return 0;
}

void my_callback_on_key_arrival(char c){

}