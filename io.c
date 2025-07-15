#include "io.h"
#include "code.h"
#include "endian.h"
#include <stdio.h>
#include <unistd.h>

uint64_t total_syms = 0; // count the symbols processed
uint64_t total_bits = 0; // count the bits processed
uint64_t read_pointer = 0;
uint8_t block[BLOCK];

uint8_t read_next_bit_from_block(int infile) {
	if (read_pointer == total_bits) {
        int bytes_read = read_bytes(infile, block, BLOCK);
        total_bits = bytes_read * 8;
        read_pointer = 0;
    }

    uint32_t byte_index = read_pointer / 8;
    uint32_t bit_index  = read_pointer % 8;

    read_pointer++;
    return (block[byte_index] >> bit_index) & 1;
}

void write_next_bit_to_block(int outfile, uint8_t bit) {
    uint32_t byte_index = total_bits / 8;
    uint32_t bit_index  = total_bits % 8;

    block[byte_index] = block[byte_index] | (bit << bit_index);
	
	total_bits++;

    if (total_bits == BLOCK * 8) {
        // block is full, write the block to file and clear it's contents
        flush_pairs(outfile);

        for(uint32_t i = 0; i < BLOCK; i++) {
            block[i] = 0;
        }
        total_bits = 0;
    }
}

int read_bytes(int infile, uint8_t *buf, int to_read){
	int bytes_read = read(infile, buf, to_read);
//    for(int32_t i = bytes_read; i < to_read; i++) {
//        buf[i] = 0;
//    }
    return bytes_read;
}

int write_bytes(int outfile, uint8_t *buf, int to_write){
	return write(outfile, buf, to_write);
}

void read_header(int infile, FileHeader *header){
	size_t header_size = sizeof(FileHeader);
    read(infile, header, header_size);

    if (big_endian()) {
        header->magic = swap32(header->magic);
        header->protection = swap16(header->protection);
    }
}

void write_header(int outfile, FileHeader *header){
	if (big_endian()) {
		header->magic = swap32(header->magic);
		header->protection = swap16(header->protection);
	}
	size_t header_size = sizeof(FileHeader);
    write(outfile, header, header_size);
}

bool read_sym(int infile, uint8_t *sym){
	return read_bytes(infile, sym, 1);
}

void write_pair(int outfile, uint16_t code, uint8_t sym, int bitlen){
	for (uint8_t i = 0; i < bitlen; i++) {
        write_next_bit_to_block(outfile, (code >> i) & 1);
    }

    for (uint8_t i = 0; i < 8; i++) {
        write_next_bit_to_block(outfile, (sym >> i) & 1);
    }
}

void flush_pairs(int outfile){
	uint32_t num_bytes = total_bits / 8;
    if (total_bits % 8) { 
		num_bytes++;
	}
    write_bytes(outfile, block, num_bytes);
    fsync(outfile);
}

bool read_pair(int infile, uint16_t *code, uint8_t *sym, int bitlen){
	*code = 0;
    for (uint8_t i = 0; i < bitlen; i++) {
        *code = *code | (read_next_bit_from_block(infile) << i);
    }

    *sym = 0;
    for (uint8_t i = 0; i < 8; i++) {
        *sym = *sym | (read_next_bit_from_block(infile) << i);
    }
    return (*code != STOP_CODE);	
}

void write_word(int outfile, Word *w){
	write(outfile, w->syms, w->len);
}

void flush_words(int outfile){
	fsync(outfile);
}

