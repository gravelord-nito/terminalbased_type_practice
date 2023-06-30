# TTP: terminal based typing practice
Is a simple game to improve typing speed(complete random key pressing) written in C.

Final project of Basic Programming university course at [IUT](https://english.iut.ac.ir/).

## Usage
Clone the repo and execute main.c

## Description
You must login or register before playing the game. There will be 3 save slots for each user which saves the player's difficulty, score, winstreak and last time played. For playing a new game you could choose one to continue or overwrite one of these slots. When continuing a saved slot, current hitted score will be added to the total score of that slot. If you lose your save in that slot will be removed.

There are a couple of waves for each difficulty played. Words will come from above the screen one by one with a fixed interval each wave. By default you'll be typing the updown most word you see, but you could also move up or down using arrow keys to type another word shown. Each letter typed, if correct, will become red, else will be yellow and you'll have to backspace and correct your errors in order to finish the word and get it's score.

There are 3 types of words:

  1. Normal: is keyboard letters and numbers between 1 to 10 length with score of 1
  2. Long: is keyboard letters and numbers between 11 to 20 length with score of 2
  3. Hard: is of length 1 to 20 and may contain some weirder characters with score of 3
  
Also there are some vague words which don't show the word to you untill you move on them which it's score will be one more than the type it is.

There also is 3 difficulty modes:
  1. Easy: Including 6 waves with initial time interval 10 seconds and decrement of 80%. Long words may come from wave 5 and words may become vague from wave 4.
  2. Medium: Including 5 waves with initial time interval 8 seconds and decrement of 70%. Long words may come from wave 4, Hard words from wave 5 and words won't be vague.
  3. Easy: Including 4 waves with initial time interval 5 seconds and decrement of 60%. Long words may become from wave 2, Hard words from wave 3 and words may be vague at all times.
  
You can play one handed( left or right ) or both, you can play lowercase only or both lowercase and uppercase too (in this mode each word will have one extra score point).

## Customization
Almost all global variables are changable without any harm. By changing HEIGHT you could make the game easier or harder.

## Notable bonus codded stuff
* Added a hash function to save passwords more securely.
* Coded the linked list two-ways so that the player can move up and down( with arrow keys ) and type whichever word he wants, not just the last one.
* One handed mode, lowercase/ uppercase mode
* Pushed the project to github
* Backspace and correction of errors
* Few changes and extra parameters to make the game more logical and playable
