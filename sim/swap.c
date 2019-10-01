#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE_BUF 4096

char buf[SIZE_BUF+3];
char revbuf[SIZE_BUF+3];

int main(int argc, char **argv)
{
  char outfilename[128];
  if (argc != 3) {
    if (argc != 2) {
      printf("USAGE: %s {{input}} {{output}}\n", argv[0]);
      exit(1);
    }
    else {
      printf("saving to %s.big", argv[1]);
      int i=0;
      while (argv[1][i] != '\0') {
        outfilename[i] = argv[1][i];
        i++;
      }
      outfilename[i++] = '.';
      outfilename[i++] = 'b';
      outfilename[i++] = 'i';
      outfilename[i++] = 'g';
      outfilename[i++] = '\0';
    }
  }
  else strcpy(outfilename, argv[2]);


  FILE *fin, *fout;
  fin = fopen(argv[1], "rb");
  //fout = fopen(argv[2], "wb+");
  fout = fopen(outfilename, "wb+");

  while (1) {
    if (feof(fin)) break;
    else {
      int size = fread(buf, 1, SIZE_BUF, fin);
      int rem = size % 4;
      if (rem) {
        for (int i=0; i<4; i++) if (size+i < SIZE_BUF) buf[size+i] = 0;
        size += 4 - rem;
      }
      for (int i=0; i<size; i+=4) {
        revbuf[i+0] = buf[i+3];
        revbuf[i+1] = buf[i+2];
        revbuf[i+2] = buf[i+1];
        revbuf[i+3] = buf[i+0];
      }
      fwrite(revbuf, 1, size, fout);
    }
  }

  fclose(fin);
  fclose(fout);

  return 0;
}
