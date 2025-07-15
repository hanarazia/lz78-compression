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
    printf("\tCompresses files using the LZ78 compression algorithm.\n");
    printf("\tCompressed files are decompressed with the corresponding decoder.\n");

    printf("USAGE\n");
    printf("\t./encode [-vh] [-i input] [-o output]\n\n");

    printf("OPTIONS\n");
    printf("\t-v          Display compression statistics\n");
    printf("\t-i input    Specify input to compress (stdin by default)\n");
    printf("\t-o output   Specify output of compressed input (stdout by default)\n");
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


void compress(int infile, int outfile) {
	TrieNode *root = trie_create();
	TrieNode *curr_node = root;
	TrieNode *prev_node = NULL;
	uint8_t curr_sym = 0;
	uint8_t prev_sym = 0;
	uint16_t next_code = START_CODE;
	
	while (read_sym(infile, &curr_sym)) { 
		TrieNode *next_node = trie_step(curr_node, curr_sym);
		if (next_node != NULL) {
			prev_node = curr_node;
			curr_node = next_node;
		}
		else {
			write_pair(outfile, curr_node->code, curr_sym, bit_length(next_code));
			curr_node->children[curr_sym] = trie_node_create(next_code);
			curr_node = root;
			next_code = next_code + 1;
		}
		if (next_code == MAX_CODE) {
			trie_reset(root);
			curr_node = root;
			next_code = START_CODE;
		}
		prev_sym = curr_sym;
	}
	
	if (curr_node != root) {
		write_pair(outfile, prev_node->code, prev_sym, bit_length(next_code));
		next_code = (next_code+1) % MAX_CODE;
	}

	write_pair(outfile, STOP_CODE, 0, bit_length(next_code));
	flush_pairs(outfile);
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
	header.magic = MAGIC;
	header.protection = sb.st_mode;
	write_header(output_fd, &header);

	compress(input_fd, output_fd);

	uint32_t input_filesize = sb.st_size;
	
	if (fstat(output_fd, &sb) == -1) {
        printf("fstat() failed on %s file.\n", output_file_name);
        exit(1);
    }
	uint32_t output_filesize = sb.st_size;

	if (verbose) {
		printf("Compressed file size: %d bytes\n", output_filesize); 
		printf("Uncompressed file size: %d bytes\n", input_filesize);
		float ratio = (input_filesize - output_filesize) * 100.0;
		ratio = ratio / input_filesize;
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
