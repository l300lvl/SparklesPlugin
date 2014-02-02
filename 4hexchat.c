#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if(argc < 3){puts("Syntax: 4Hexchat Input Output"); return -1;}

  FILE *File = fopen(argv[1],"rb");
  if(File == NULL) {
    fputs("Couldn't open file for reading",stderr); return 0;
  }
  fseek(File, 0, SEEK_END);
  long FileSize = ftell(File);
  rewind(File);
  char *Buffer = (char*)malloc(sizeof(char)*FileSize);
  if(Buffer == NULL) {
    fclose(File);
    fputs("Can't allocate memory",stderr); return 0;
  }
  if(FileSize != fread(Buffer,1,FileSize,File)) {
    fclose(File);
    fputs("Can't read from file",stderr); return 0;
  }
  fclose(File);
  File = fopen(argv[2],"wb");
  if(File == NULL) {
    fputs("Couldn't open file for writing",stderr); return 0;
  }

  int i;
  for(i=0;i<FileSize;i++) {
    if(!memcmp(&Buffer[i], "xchat_", 6) && Buffer[i-1] != 'e') fprintf(File, "he");
    if(!memcmp(&Buffer[i], "xchat-", 6) && Buffer[i-1] != 'e') fprintf(File, "he");
    if(!memcmp(&Buffer[i], "XCHAT_", 6) && Buffer[i-1] != 'E') fprintf(File, "HE");
    fputc(Buffer[i], File);
  }
  fclose(File);
  free(Buffer);
  return 1;
}
