#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

enum SEQ_PREFERENCE {
  UNSPECIFIED,
  PRIMARY,
  SECONDARY
};

void copy_file(char* in, char* out);
void print_help();

int main(int argc, char* argv[]) {
  puts("regfixup 1.1 (https://github.com/helloryuko/regfixup)");

  if (argc < 2) {
    print_help();
    return 0;
  }

  char* hive_path = "\0";
  enum SEQ_PREFERENCE seq_pref = UNSPECIFIED;
  bool checksum_only = false;

  FILE* hive_file;
  size_t hive_file_size;
  char* header_data;
  bool fixed_seq = false;
  bool fixed_checksum = false;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-p") == 0)
      seq_pref = PRIMARY;
    else if (strcmp(argv[i], "-s") == 0)
      seq_pref = SECONDARY;
    else if (strcmp(argv[i], "-c") == 0)
      checksum_only = true;
    else if (strcmp(argv[i], "-h") == 0) {
      print_help();
      return 0;
    }
    else if (argv[i][0] == '-') {
      printf("error: invalid parameter: \"%s\"", argv[i]);
      return 3;
    }
    else {
      hive_path = argv[i];
    }
  };

  if (hive_path[0] == '\0') {
    printf("error: no hive path provided\n");
    return 3;
  }

  hive_file = fopen(hive_path, "rb");
  if (hive_file == NULL) {
    printf("error: can't open file (%s)\n", strerror(errno));
    return 1;
  }

  header_data = calloc(512, sizeof(char));
  fread(header_data, sizeof(char), 512, hive_file);
  fclose(hive_file);

  if (
    header_data[0] != 'r' ||
    header_data[1] != 'e' ||
    header_data[2] != 'g' ||
    header_data[3] != 'f'
  ) {
    printf("error: file provided isn't a regfile\n");
    return 2;
  }

  if (!checksum_only) {
    int32_t* primary_seq = (int32_t*) (header_data + 4);
    int32_t* secondary_seq = (int32_t*) (header_data + 8);
  
    if (*primary_seq == *secondary_seq) {
      printf("info: sequences are the same\n");
    }
    else {
      printf("info: sequences are not matching (primary: %d, secondary: %d)\n", *primary_seq, *secondary_seq);
      
      if (seq_pref == UNSPECIFIED) {
        printf("choose which sequence to prefer (1 or 2)\n");
        printf("> ");

        ask:
        const char chosen = getchar();
	
	if (chosen == '1') {
          seq_pref = PRIMARY;
	}
	else if (chosen == '2') {
          seq_pref = SECONDARY;
	}
	else if (chosen == '\n') {
          goto ask;
	}
	else {
          puts("invalid option");
          printf("> ");
	  goto ask;
	}
      }

      if (seq_pref == PRIMARY) {
        *secondary_seq = *primary_seq;
      }
      else if (seq_pref == SECONDARY) {
        *primary_seq = *secondary_seq;
      }

      fixed_seq = true;
      printf("info: synchronized sequences\n");
    }
  }

  int32_t* original_checksum = (int32_t*) (header_data + 508);
  int32_t new_checksum = *((int32_t*) (header_data)); // first number

  for (int offset = 4; offset < 508; offset += 4) {
    new_checksum ^= *((int32_t*) (header_data + offset));
  }

  if (*original_checksum == new_checksum) {
    printf("info: checksum is correct (%d)\n", new_checksum);
  }
  else {
    printf("info: recalculated checksum (old: %d, new: %d)\n", *original_checksum, new_checksum);
    *original_checksum = new_checksum;
    fixed_checksum = true;
  }

  if (!fixed_checksum || !fixed_seq) {
    printf("nothing to fix! the hive wasn't edited.\n");
  }
  else {
    const char* backup_postfix = ".bad";
    
    char* buffer = calloc(strlen(hive_path) + strlen(backup_postfix) + 1, sizeof(char));
    buffer[0] = '\0';

    strcat(buffer, hive_path);
    strcat(buffer, backup_postfix);

    copy_file(hive_path, buffer);
    free(buffer);

    hive_file = fopen(hive_path, "rb+");
    fseek(hive_file, 0, SEEK_SET);
    fwrite(header_data, 1, 512, hive_file);
    fclose(hive_file);

    printf("info: fix written successfully!\n");
  }
};

void print_help() {
  puts("fix broken registry hives with desynchronized sequence numbers or wrong checksum\n"
       "\n"
       "usage: regfixup [hive file location]\n"
       "\n"
       "optional parameters:\n"
       "  -h  show this help message\n"
       "  -p  prefer primary sequence number\n"
       "  -s  prefer secondary sequence number\n"
       "  -c  only recalculate checksum\n"
       "\n"
       "return codes:\n"
       "  0   fixup successful or not required\n"
       "  1   can't open file\n"
       "  2   file isn't a regfile\n"
       "  3   invalid parameters");
}

void copy_file(char* in, char* out) {
  const char* copy_command = (
#if defined(_WIN64) || defined(_WIN32)
    "copy"
#else
    "cp"
#endif
  );

  char* buffer = calloc(strlen(copy_command) + 1 + strlen(in) + 1 + strlen(out) + 1, sizeof(char));
  buffer[0] = '\0';

  strcat(buffer, copy_command);
  strcat(buffer, " ");
  strcat(buffer, in);
  strcat(buffer, " ");
  strcat(buffer, out);

  printf("info: running command \"%s\"\n", buffer);
  system(buffer);

  free(buffer);
};

