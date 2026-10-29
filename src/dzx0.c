/*
 * ZX0 decompressor - by Einar Saukas
 * https://github.com/einar-saukas/ZX0
 */

#include <setjmp.h>

#define BUFFER_SIZE 32000 /* must be > MAX_OFFSET */
#define INITIAL_OFFSET 1

#define FALSE 0
#define TRUE 1

unsigned char *input_data;
unsigned char *output_data;
unsigned int input_index; 
unsigned int output_index;
unsigned int output_limit ;
int bit_mask;
int bit_value;
int backtrack;
int last_byte;

jmp_buf jump_buffer;

int read_byte() {
  last_byte = input_data[input_index++];
  return last_byte;
}

int read_bit() {
  if (backtrack) {
    backtrack = FALSE;
    return last_byte & 1;
  }
  bit_mask >>= 1;
  if (bit_mask == 0) {
    bit_mask = 128;
    bit_value = read_byte();
  }
  return bit_value & bit_mask ? 1 : 0;
}

int read_interlaced_elias_gamma(int inverted) {
  int value = 1;
  while (!read_bit()) {
    value = value << 1 | read_bit() ^ inverted;
  }
  return value;
}

void write_byte(int value) {
  output_data[output_index++] = (unsigned char)value;
  if (output_index >= output_limit) {
    longjmp(jump_buffer, 1);
  }
}
void write_bytes(int offset, int length) {
  int i;
  while (length-- > 0) {
    i = output_index - offset;
    write_byte(output_data[i >= 0 ? i : BUFFER_SIZE + i]);
  }
}

int dzx0_decompress(unsigned char *input, unsigned char *output,
                    unsigned int limit) {
  output_limit = limit;
  int last_offset = INITIAL_OFFSET;
  int length;
  int i;
  input_data = input;
  output_data = output;
  input_index = 0;
  output_index = 0;
  bit_mask = 0;
  bit_value = 0;
  backtrack = FALSE;
  if (setjmp(jump_buffer) != 0) {
    return output_index;
  }

COPY_LITERALS:
  length = read_interlaced_elias_gamma(FALSE);
  for (i = 0; i < length; i++) {
    write_byte(read_byte());
  }
  if (read_bit())
    goto COPY_FROM_NEW_OFFSET;

  /*COPY_FROM_LAST_OFFSET:*/
  length = read_interlaced_elias_gamma(FALSE);
  write_bytes(last_offset, length);
  if (!read_bit())
    goto COPY_LITERALS;

COPY_FROM_NEW_OFFSET:
  last_offset = read_interlaced_elias_gamma(FALSE);
  if (last_offset == 256) {
    return output_index;
  }
  last_offset = last_offset * 128 - (read_byte() >> 1);
  backtrack = TRUE;
  length = read_interlaced_elias_gamma(FALSE) + 1;
  write_bytes(last_offset, length);
  if (read_bit())
    goto COPY_FROM_NEW_OFFSET;
  else
    goto COPY_LITERALS;
}
