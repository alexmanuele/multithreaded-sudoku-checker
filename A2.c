#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_THREADS 27 //27 threads will be created

/*The following are external variables which are accessed by 
  multipe threads and functions. */

int values[9][9]; //A 9x9 array to store sudoku values
int results[NUM_THREADS]; //array of booleans for each thread

/* structure for thread data. Each thread will be initialized with a
   tid, row and column to start from, and a function number.
   Threads with function number 1 will check rows.
   Threads with function number 2 will check columns.
   Threads with function number 3 will check subgrids.
*/

struct parameters{
  int row;
  int column;
  int tid;
  int function;
};
//use tuple for upper left corners of subgrids
struct tuple{
  int row;
  int column;
};
//Methods for checking rows, columns, subgrids
void checkSubGrids(struct parameters* start, int results[27], int values[9][9], int this);
void checkRow(struct parameters* start, int results[27], int values[9][9], int this);
void checkColumn(struct parameters* start, int results[27], int values[9][9], int this);
/*The method each thread will be passed to. It will subsequently
  call the appropriate function as indicated by the thread data.
*/
void *threadFunction(void *arg);

int main(int argc, char *argv[]){
  pthread_t threads[NUM_THREADS]; //array of threads
  struct parameters thread_data[NUM_THREADS]; //data
  int threadCheck; //used to check system call
  //vars for reading the file
  
  FILE *f;
  char *line = NULL;
  size_t len = 0; 
  ssize_t read;
  int i = 0; //index for scanning the input file
  int temp; //temp variable to read input from file

  //create an array containing the co-ordinates for subgrids
  struct tuple gridCorners[9] =
    {{0, 0}, {0, 3}, {0, 6},
     {3, 0}, {3, 3}, {3, 6},
     {6, 0}, {6, 3}, {6,6}};
  
  //check if the command has been entered correctly
  if(argc != 2){
    printf("Incorrect number of command arguments:\n");
    printf("Use format ./A2 filename\n");
    return 1;
  }
  //Open the file; check if the system call was successful
  f = fopen(argv[1], "r");
  if (f==NULL){ //check success of system call
    perror("FileOpen Failed: ");
    return 1;
  }
  /*The following loop reads the integers from the file into a 9x9
    array. It first grabs a line from the file using the getline()
    function and checks if the length of the line is correct.
    Then, it uses sscanf() to extract each integer from the line.
  */
  while (( read = getline(&line, &len, f)) != -1){
    if(read > 19){
      printf("Error: Incorrect file format. Input should be a 9x9 sudoku solution, with each entry seperated by a whitespace.\n");
	exit(EXIT_FAILURE);
    }
    temp = sscanf(line, "%d%d%d%d%d%d%d%d%d",&values[i][0],&values[i][1],&values[i][2],&values[i][3],&values[i][4],&values[i][5],&values[i][6],&values[i][7],&values[i][8]);
    if (temp == EOF){
      printf("Error: Incorrect File format\n");
      exit(EXIT_FAILURE);
    }
    i++;
  }
  free(line);
  fclose(f);

  /*Create threads: 1 for each row, 1 for each column, 1 for
    each subgrid, 27 total */
  //This loop creates row and column threads
  for( i=0; i < NUM_THREADS; i++){
    thread_data[i].tid = i;
    if(i < 9){ //first nine threads will check rows
      thread_data[i].row = i;
      thread_data[i].column = 0;
      thread_data[i].function = 1; //1 -> checkRow()
      threadCheck = pthread_create(&threads[i], NULL, threadFunction, &thread_data[i]);
      if(threadCheck == -1){
	printf("Error: pthread_create\n");
	exit(EXIT_FAILURE);
      }
    }else if( i < 18){ //next nine threads check columns
      thread_data[i].column = i - 9;
      thread_data[i].row = 0;
      thread_data[i].function = 2; //2-> checkColumn()
      threadCheck = pthread_create(&threads[i], NULL, threadFunction, &thread_data[i]);
      if(threadCheck == -1){
	printf("Error: pthread_create\n");
	exit(EXIT_FAILURE);
      }
    }else{
      thread_data[i].row = gridCorners[i-18].row;
      thread_data[i].column = gridCorners[i-18].column;
      thread_data[i].function = 3; //3 -> checkSubGrids()
      threadCheck = pthread_create(&threads[i], NULL, threadFunction, &thread_data[i]);
      if(threadCheck == -1){
	printf("Error: pthread_create\n");
	exit(EXIT_FAILURE);
      }
    }
  }
  //Create the threads for the subgrids
  
  //wait for threads
  for(i=0; i < NUM_THREADS; i++){
    pthread_join(threads[i], NULL);
  }
  //Loop through Results and check for success:
  for(i=0; i < NUM_THREADS; i++){
    if(results[i] == 0){
      printf("Your solution was invalid. Try again.\n");
      goto end;
    }
  }
  printf("Your solution was valid, congratulations!\n");
 end:
  exit(EXIT_SUCCESS);
  
}
/* checkSubGrids is passed the upper left corner of a subgrid, as
   well as the array storing the final result of all checks, the
   2D array representing the sudoku grid, and an value indicating
   the spot of the results array corresponding to this thread.
   it checks each row and column, recording the number of times the
   integer found at that co-ordinate occurs. If an int occurs more
   than once, the result is set to false and an error is printed. */
void checkSubGrids(struct parameters* start, int results[27], int values[9][9], int this){
  int integers[9] = {0};
  int i = 0;
  int j = 0;
  results[this] = 1;
  for(i = start->row; i < start->row + 3; i++){
    for(j = start->column; j < start->column + 3; j++){
      integers[values[i][j]] = integers[values[i][j]] + 1;
      if(integers[values[i][j]] > 1){
	printf("Error in subgrid: Row %d, Column %d\n", i+1, j+1);
	results[this] = 0;
      }
    }
  }
}
//checkRow and checkColumn work the same as check subGrid but only 1D
void checkRow(struct parameters* start, int results[27], int values[9][9], int this){
  int integers[9] = {0};
  int rowNum = start->row;
  int i = 0;
  results[this] = 1;
  for(i = 0; i < 9; i++){ //check every column in the row
    integers[values[rowNum][i]] = integers[values[rowNum][i]] + 1;
    if(integers[values[rowNum][i]] > 1){
      printf("Error in Row: Row %d, Column %d\n",rowNum+1, i+1);
      results[this] = 0;
    }
  }
}
void checkColumn(struct parameters* start, int results[27], int values[9][9], int this){
  int integers[9] = {0};
  int colNum = start->column;
  int i = 0;
  results[this] = 1;
  for(i = 0; i < 9; i++){ //check every row in the column
    integers[values[i][colNum]] = integers[values[i][colNum]] + 1;
    if(integers[values[i][colNum]] > 1){
      printf("Error in Column: Row %d, Column %d\n", i+1, colNum+1);
      results[this] = 0;
    }
  }
}
/*Thread function. Accepts the thread data, then passes it to the
  appropriate sub-problem function. */
void *threadFunction(void *arg){
  struct parameters *data = (struct parameters *)arg;
  int function = data->function;
  int this = data->tid;
  if(function == 1){
    checkRow(data, results, values, this);
  }
  else if(function == 2){
    checkColumn(data, results, values, this);
  }
  else if(function == 3){
    checkSubGrids(data, results, values, this);
  }
  pthread_exit(NULL);
}
  
