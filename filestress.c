#include <stdio.h>
#include <stdlib.h>


int main()
{

  FILE * fp;
  int i;
while(1)
    {
        fp = fopen ("file.txt", "w+");
        for(i = 0; i < 100000; i++) fprintf(fp, "yuri");
        fclose(fp);
        remove("file.txt");
     }
     return(0);
}

   