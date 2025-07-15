#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "io.h"
#include "trie.h"
#include "code.h"
#include <math.h>

#define OPTIONS "i:o:vh"

void help() {
    printf("SYNOPSIS\n");
    printf("\tDecompresses files using the LZ78 compression algorithm.\n");
    printf("\tDecompressed files are compressed with the corresponding encoder.\n");

    printf("USAGE\n");
    printf("\t./decode [-vh] [-i input] [-o output]\n\n");

    printf("OPTIONS\n");
    printf("\t-v          Display decompression statistics\n");
    printf("\t-i input    Specify input to decompress (stdin by default)\n");
    printf("\t-o output   Specify output of decompressed input (stdout by default)\n");
    printf("\t-h          Display program help and usage\n");
    exit(0);
}

int32_t bit_length(uint16_t number) {
	if (number == 0) {
		return 1;
	}
	// ⌊log2(x)⌋+1
	return log2(number) + 1;
}

void decompress(int infile, int outfile) {
	WordTable *table = wt_create();
	uint8_t curr_sym = 0;
	uint16_t curr_code = 0;
	uint16_t next_code = START_CODE;
	
	while (read_pair(infile, &curr_code, &curr_sym, bit_length(next_code))) {
		table[next_code] = word_append_sym(table[curr_code], curr_sym); 
		write_word(outfile, table[next_code]);
		next_code = next_code + 1;
		if (next_code == MAX_CODE) {
			wt_reset(table);
			next_code = START_CODE; 
		}
	}
	flush_words(outfile);
}

int main(int argc, char ** argv) {
	int opt;
    char *input_file_name = NULL;
    char *output_file_name = NULL;
    int verbose = 0;

	while ((opt = getopt (argc, argv, OPTIONS)) != -1) {
        switch (opt) {
            case 'i': {
                input_file_name = optarg;
                break;
            }

            case 'o': {
                output_file_name = optarg;
				break;
            }

			case 'v': {
                verbose = 1;
                break;
            }

			case 'h': {
        		help();
			}
        }
    }	

	int input_fd;
	if (input_file_name == NULL) {
		input_fd = STDIN_FILENO;
	}
	else {
		input_fd = open(input_file_name, O_RDONLY);
		if (input_fd == -1) {
			printf("Could not read %s file.\n", input_file_name);
        	exit(1);
		}
	}
	
	int output_fd;
    if (output_file_name == NULL) {
        output_fd = STDOUT_FILENO;
    }
    else {
        output_fd = creat(output_file_name, S_IRUSR | S_IWUSR);
        if (output_fd == -1) {
            printf("Could not open %s file.\n", output_file_name);
            exit(1);
        }
    }
    
	struct stat sb;
	if (fstat(input_fd, &sb) == -1) {
		printf("fstat() failed on %s file.\n", input_file_name);
		exit(1);
	}

	fchmod(output_fd, sb.st_mode);
	
	FileHeader header;
	read_header(input_fd, &header);
	
	if (header.magic != MAGIC) {
		printf("Magic word = %X, expecting %X.\n", header.magic, MAGIC);
		exit(1);
	}

	decompress(input_fd, output_fd);

	uint32_t input_filesize = sb.st_size;
	
	if (fstat(output_fd, &sb) == -1) {
        printf("fstat() failed on %s file.\n", output_file_name);
        exit(1);
    }
	uint32_t output_filesize = sb.st_size;

	if (verbose) {
		printf("Uncompressed file size: %d bytes\n", output_filesize); 
		printf("Compressed file size: %d bytes\n", input_filesize);
		float ratio = (output_filesize - input_filesize) * 100.0;
        ratio = ratio / output_filesize;
		printf("Space saving: %.2f%%\n", ratio); 

	}

	if (input_fd != STDIN_FILENO) {
		close(input_fd);
	}
	if (output_fd != STDOUT_FILENO) {
		close(output_fd);
	}

	return 0;

}
