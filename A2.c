#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

int main(int argc, char *argv[]){
  int values[9][9]; //A 9x9 array to store the sudoku values
  int results[11]; //array of booleans for each region of sudoku
  FILE *f;
  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int i = 0; //index for scanning the input file
  int temp; //temp variable to read input from file
  //check if the command has been entered correctly
  if(argc != 2){
    printf("Incorrect number of command arguments:\n");
    printf("Use format ./A2 filename\n");
    return 1;
  }
  f = fopen(argv[1], "r");
  if (f==NULL){ //check success of system call
    perror("FileOpen Failed: ");
    return 1;
  }
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
    printf("%d", values[i][0]);
    i++;
  }
  free(line);
  exit(EXIT_SUCCESS);
}
