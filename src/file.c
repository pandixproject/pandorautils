#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static const char *file_type(mode_t mode) {
  if (S_ISREG(mode))
    return "regular file";
  if (S_ISDIR(mode))
    return "directory";
  if (S_ISLNK(mode))
    return "symbolic link";
  if (S_ISCHR(mode))
    return "character device";
  if (S_ISBLK(mode))
    return "block device";
  if (S_ISFIFO(mode))
    return "FIFO";
  if (S_ISSOCK(mode))
    return "socket";
  return "unknown";
}

static const char *content_type(const char *path) {
  FILE *file;
  unsigned char buffer[512];
  size_t size;
  size_t i;
  int text = 1;

  file = fopen(path, "rb");
  if (file == NULL)
    return NULL;

  size = fread(buffer, 1, sizeof(buffer), file);
  fclose(file);

  if (size >= 4 && buffer[0] == 0x7f && buffer[1] == 'E' && buffer[2] == 'L' &&
      buffer[3] == 'F')
    return "ELF executable";

  if (size >= 4 && buffer[0] == 0x50 && buffer[1] == 0x4b &&
      buffer[2] == 0x03 && buffer[3] == 0x04)
    return "ZIP archive";

  if (size >= 3 && buffer[0] == 0x1f && buffer[1] == 0x8b)
    return "gzip compressed data";

  if (size >= 4 && buffer[0] == 0x25 && buffer[1] == 0x50 &&
      buffer[2] == 0x44 && buffer[3] == 0x46)
    return "PDF document";

  if (size >= 8 && memcmp(buffer, "\x89PNG\r\n\x1a\n", 8) == 0)
    return "PNG image";

  if (size >= 3 && buffer[0] == 0xff && buffer[1] == 0xd8 && buffer[2] == 0xff)
    return "JPEG image";

  for (i = 0; i < size; i++) {
    if (buffer[i] == 0)
      return "data";

    if (buffer[i] < 32 && buffer[i] != '\n' && buffer[i] != '\r' &&
        buffer[i] != '\t') {
      text = 0;
      break;
    }
  }

  if (text)
    return "text";

  return "data";
}

static void identify(const char *path) {
  struct stat st;
  const char *type;

  if (lstat(path, &st) == -1) {
    perror(path);
    return;
  }

  type = file_type(st.st_mode);

  if (S_ISREG(st.st_mode)) {
    const char *content = content_type(path);

    if (content != NULL)
      type = content;
  }

  printf("%s: %s\n", path, type);
}

int main(int argc, char **argv) {
  int i;

  if (argc < 2) {
    fprintf(stderr, "usage: pfile FILE...\n");
    return EXIT_FAILURE;
  }

  for (i = 1; i < argc; i++)
    identify(argv[i]);

  return EXIT_SUCCESS;
}
