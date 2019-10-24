#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curses.h>
#include <termios.h>
#include <fcntl.h>


#define ROW 10
#define COLUMN 50 
#define LENGTH 17
int gamestatus=1;
pthread_mutex_t mutex;
pthread_mutex_t mutex2;
pthread_cond_t threshold_cv;
struct Node{
	int x , y; 
	Node( int _x , int _y ) : x( _x ) , y( _y ) {}; 
	Node(){} ; 
} frog ; 

char map[ROW+1][COLUMN]; 

// Determine a keyboard is hit or not. If yes, return 1. If not, return 0. 
int kbhit(void){
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);

	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);

	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);

	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if(ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
}

/*
 * Function: printMap()
 * -------------------------------------
 * Description: Used to print the map to console
 * Implementation notes: loop to print each element
 * in char[][]
 */

void printMap(char[11][50]){
	printf("\033[H");
		for (int i = 0; i <= ROW; i++){
			for (int j = 0; j < COLUMN; j++)
			{
				printf("%c", map[i][j]);
			}
			printf("\n");
		}
}

/*
 * Function: logs_move()
 * ------------------------------------
 * Description: used to implement multithreading. 
 * This function will initialize one log, receive
 * keyboard hits and refresh the image showed in
 * the console.
 * Inplementation notes: use while loop to move
 * the log continuously. Check the game status,
 * the while loop will break if the integer gamestats
 * equals to 0.
 */

void *logs_move( void *t ){
	int tid = (long)t;
	int y = tid+1;

	pthread_mutex_lock(&mutex);	//lock the map

	/*  initialize the log  */

	for (int i = 0; i < LENGTH; i++)
		map[y][i] = '=';
	
	for (int k= LENGTH; k < COLUMN; k++)
		map[y][k] = ' ';
	
	printMap(map);

	int temp[COLUMN];	//used to store row temporarily
	int v=(y % 2 == 1)?1:49;
	int j = 0;
	pthread_mutex_unlock(&mutex);
	
	while(gamestatus==1){
		
		pthread_mutex_lock(&mutex);
		pthread_mutex_lock(&mutex2);

	 	/*  Move the logs  */
		
		for (j = 0; j < COLUMN; j++){
			if (y != frog.x) {
				if(map[y][j] == '0') temp[(j+v)%COLUMN] = '=';
				else temp[(j+v)%COLUMN] = map[y][j];
			}
			else {
				//detact wether the frog is on the log
				//correct the map if anything goes wrong
				
				if (j != frog.y && map[y][j] == '0') temp[(j+v)%COLUMN] = '=';
				else if(j == frog.y && map[y][j] != '0') temp[(j+v)%COLUMN] = '0';
				//move the log
				else temp[(j+v)%COLUMN] = map[y][j];
			}
			
		}
		for (j = 0; j < COLUMN; j++){
			
			map[y][j] = temp[j];	
		}
			
		/*  Check keyboard hits, to change frog's position or quit the game. */
		if( kbhit() ){
			char dir = getchar();
			
			/*  Quit the game  */
			if (dir == 'q' || dir == 'Q') {
				gamestatus = 0;	//0 stands for quit
			}
			/*  Go up  */
			if (dir == 'w' || dir == 'W') {
				
				if (map[frog.x-1][frog.y] == '='|| map[frog.x-1][frog.y] == '|'){
					map[frog.x][frog.y] = (frog.x==ROW)?'|':'=';
					
					int temp = frog.x-1;
					frog.x = temp;
					map[frog.x][frog.y] = '0';
					
				}
				//jump into the river
				else gamestatus = 3;
			}
			/*  Go down  */
			if (dir == 's' || dir == 'S') {
				if (map[frog.x+1][frog.y] == '='|| map[frog.x+1][frog.y] == '|'){
					map[frog.x][frog.y] = (frog.x==ROW)?'|':'=';
					int temp = frog.x+1;
					frog.x = temp;
					map[frog.x][frog.y] = '0';
				}
				else gamestatus = 3;
			}
			/*  Go left  */
			if (dir == 'a' || dir == 'A') {
				if (map[frog.x][frog.y-1] == '=' || map[frog.x][frog.y-1] == '|'){
					map[frog.x][frog.y] = (frog.x==ROW)?'|':'=';
					int temp = frog.y-1;
					frog.y = temp;
					map[frog.x][frog.y] = '0';
				}
				else gamestatus = 3;
			}
			/*  Go right  */
			if (dir == 'd' || dir == 'D') {
				if (map[frog.x][frog.y+1] == '=' || map[frog.x][frog.y+1] == '|'){
					map[frog.x][frog.y] = (frog.x==ROW)?'|':'=';
					int temp = frog.y + 1;
					frog.y = temp;
					map[frog.x][frog.y] = '0';
				}
				else gamestatus = 3;
			}
			
		}
		/*  Check game's status  */
			if (frog.x == 0) {
				gamestatus = 2;			//win the game
				
			}
			if (frog.x == y){
				if (map[frog.x][0] == '='||map[frog.x][COLUMN-1] == '=') 
				
				gamestatus = 4;			//touch the boundary

			}
			

		/*  Print the map on the screen  */

			//move the frog's position so that it move
			//along with the log it si on
			if (y == frog.x) frog.y = (frog.y+v)%COLUMN;

			printMap(map);
			//printf("(%d,%d)",frog.x,frog.y);
			pthread_mutex_unlock(&mutex2);
			pthread_mutex_unlock(&mutex);
			usleep(200000);
			if (y == 2||y == 7) usleep(400000);
			else if (y == 4|| y == 5) usleep(300000);
	}

	
	pthread_exit(NULL);
	
}

int main( int argc, char *argv[] ){

	// Initialize the river map and frog's starting position
	printf("\033[H\033[2J");
	int i , j ; 
	for( i = 1; i < ROW; ++i ){	
		for( j = 0; j < COLUMN - 1; ++j )	
			map[i][j] = ' ' ;  
	}	

	for( j = 0; j < COLUMN - 1; ++j )	
		map[ROW][j] = map[0][j] = '|' ;

	for( j = 0; j < COLUMN - 1; ++j )	
		map[0][j] = map[0][j] = '|' ;

	frog = Node( ROW, (COLUMN-1) / 2 ) ; 
	map[frog.x][frog.y] = '0' ; 
	
	//Print the map into screen
	printMap(map);


	/*  Create pthreads for wood move and frog control.  */
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&mutex2, NULL);
	pthread_cond_init(&threshold_cv, NULL);
	pthread_attr_t attr;
	pthread_t threads[ROW-1];
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    int rc, rc1;

    for(long i =0; i<ROW-1; i++){
        rc = pthread_create(&threads[i], &attr, logs_move, (void*)i);
        if(rc){
            printf("ERROR: return code from pthread_create() is %d", rc);
            exit(1);
        }
    }
	for(long i =0; i<ROW-1; i++){
        rc1 = pthread_join(threads[i], NULL);;
        if(rc1){
            printf("ERROR: return code from pthread_join() is %d", rc1);
            exit(1);
        }
    }
	/*  Display the output for user: win, lose or quit.  */
	switch(gamestatus){
				case 0:
				printf("\n--YOU QUIT THE GAME--\n");
				break;
				case 2: 
				printf("\n--YOU WIN THE GAME--\n");
				break;
				case 3:
				printf("\n--YOU LOSE THE GAME--\ni.e.you jump into the river.\n");
				break;
				case 4:
				printf("\n--YOU LOSE THE GAME--\ni.e.you touch the boundary.\n");
				break;
			} 
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&threshold_cv);
    
    pthread_exit(NULL);
	
	
	return 0;

}
