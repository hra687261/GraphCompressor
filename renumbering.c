#include "tools.h"

int main(int argc, char **argv)
{
  if(argc != 2){
    fprintf(stderr,"Expected One argument, the path to the file of the edge list \n\
(pair of positive numbers on everyline, separated by a space, and sorted with by the first and second column)\n");
    return EXIT_FAILURE;
  }
  renumbering(argv[1],0);
  return EXIT_SUCCESS;
}