#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
  if (argc < 6) {
    fprintf(stderr, "Usage: %s <input file> <output file> <frames per part> <prefix> <frame width>\n", argv[0]);
    return 1;
  }
  unsigned char framesinpart = atoi(argv[3]);
  unsigned int framewidth = atoi(argv[5]);
  unsigned int frameheight = 13;
  unsigned int total_bytes = 0;
  unsigned int parts = 0;
  unsigned int total_frames = 0;
  unsigned int max_partSize = 0;
  // find NNNVrames number
  FILE *orig = fopen(argv[1], "rb");
  if (!orig) {
    fprintf(stderr, "Failed to open original file\n");
    return 1;
  }
  fseek(orig, 0, SEEK_END);
  unsigned int orig_size = ftell(orig);
  fclose(orig);
  total_frames = orig_size / (framewidth * frameheight);
  parts = (total_frames + framesinpart - 1) / framesinpart;
  fprintf(stdout, "Total frames in original file: %d\n", total_frames);
  fprintf(stdout, "Total parts: %d\n", parts);

  FILE *fileout = fopen(argv[2], "wb");
  if (!fileout) {
    fprintf(stderr, "Failed to open output file\n");
    return 1;
  }
  fwrite(&parts, 1, 2, fileout);
  total_bytes += 2; // Account for the part count written to the file
  fwrite(&total_frames, 1, 2, fileout);
  total_bytes += 2; // Account for the frames count written to the file
fwrite(&framesinpart, 1, 1, fileout);
    fwrite(&framewidth, 1, 2, fileout);
    total_bytes += 2; 
    fwrite(&frameheight, 1, 2, fileout);
    total_bytes += 2; // Account for the frame size written to the file
  char filename[256];
    for (unsigned int i = 0; i < parts; i++) {
    sprintf(filename, "%s%03d.zx0", argv[4],  i);
    FILE *filein = fopen(filename, "rb");
    if (!filein) {
      fprintf(stderr, "Failed to open part file: %s\n", filename);
      return 1;
    }
    fseek(filein, 0, SEEK_END);
    unsigned int partsize = ftell(filein);
    if (partsize > max_partSize)
      max_partSize = partsize;
    fseek(filein, 0, SEEK_SET);
    if (i == parts - 1) {
      framesinpart = total_frames - (i * framesinpart);
    }
     fwrite(&partsize,   1, 2, fileout);
    total_bytes += 2; 
    char *buffer = (char *)malloc(partsize);
    int nread = fread(buffer, 1, partsize, filein);
    fclose(filein);
    fwrite(buffer, 1, partsize, fileout);
    free(buffer);
    total_bytes += partsize;
    fprintf(stdout, "Part %d (%d frames), size %d\n", i, framesinpart, partsize);
  
 }
  fprintf(stdout, "Total bytes written: %d\n", total_bytes);
  fprintf(stdout, "Max part size: %d\n", max_partSize);
  fclose(fileout);
  return 0;
}
