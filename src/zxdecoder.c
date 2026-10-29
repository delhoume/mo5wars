#include <cmoc.h>

#include "common.h"
#include "reader.h"
#include "zxdecoder.h"

int dzx0_decompress(const uint8_t *src, uint8_t *dst, uint16_t dst_limit);

unsigned int parts = 0;
unsigned int nframes = 0;
unsigned int current_part = 0;
unsigned int current_frame_in_part = 0;
unsigned int current_part_size = 0;
unsigned int current_num_frames = 0;
unsigned int decoded_size = 0;
unsigned int current_frame = 0;
unsigned int debugBufferSize = FRAME_BYTE_SIZE;
unsigned char frames_in_current_part = 0;
// fixed for now

unsigned char compressedFrameBuffer[MAX_COMPRESSED_PART]; // max compressed part
unsigned char decodedFrames[PART_TOTAL];

void GetFrameInfo(unsigned int *frame, unsigned int *frames) {
  *frame = current_frame;
  *frames = nframes;
}
unsigned int GetInt16() {
  unsigned int low = GetNextValue();
  unsigned int high = GetNextValue(); // Implement the function to read a 16-bit
                                      // integer from the reader
  return (high << 8) | low;
}

// #define DEBUG_ZX
#ifdef DEBUG_ZX
void showDataRow(unsigned char *data, int nvalue, unsigned char *dst_ptr) {
  for (unsigned int col = 0; col < nvalue; col++) {
    unsigned int data_index = col;
    unsigned char byte = data[data_index];
    unsigned char lowNibble = byte & 0x0F;
    dst_ptr[col * 3 + 0] =
        (byte >> 4) < 10 ? '0' + (byte >> 4) : 'A' + ((byte >> 4) - 10);
    dst_ptr[col * 3 + 1] =
        lowNibble < 10 ? '0' + lowNibble : 'A' + (lowNibble - 10);
    dst_ptr[col * 3 + 2] = ' ';
  }
}

// dest is a frame 67x13
void dumpHex(const unsigned char *data, unsigned int datasize, char *dest) {
  unsigned int visiblebytes = (SCREEN_WIDTH / TWIDTH) / 3;
  showDataRow((unsigned char *)data,
              datasize < visiblebytes ? datasize : visiblebytes,
              (unsigned char *)dest);
  memset(dest + FRAME_CHAR_WIDTH, '.', visiblebytes * 3);
  unsigned int lastrow = (datasize + visiblebytes - 1) /
                         visiblebytes; // calculate the last row to display
  showDataRow((unsigned char *)data + (lastrow - 1) * visiblebytes,
              visiblebytes, (unsigned char *)dest + 2 * FRAME_CHAR_WIDTH);
}
unsigned char debugBuffer[FRAME_BYTE_SIZE];
#endif

int ZXGetNextFrame() {
  if (parts == 0) {
    // global initialization of parts and nframes
    parts = GetInt16();
    nframes = GetInt16();
    GetNextValue(); // not used h
    GetInt16();
    GetInt16();
    current_frame = 0;
    current_frame_in_part = 0;
  }
  if (current_frame_in_part == 0) {
    // load current part
    current_part_size = GetInt16();
    for (unsigned int j = 0; j < current_part_size; j++) {
      compressedFrameBuffer[j] = GetNextValue();
    }
    decoded_size =  dzx0_decompress(compressedFrameBuffer, decodedFrames, PART_TOTAL + 1);
    // process each byte of the current part
    if (decoded_size % FRAME_BYTE_SIZE != 0) {
      // handle error: decoded size is not a multiple of frame byte size
    }
    frames_in_current_part = decoded_size / FRAME_BYTE_SIZE;
  }
  unsigned int offset_to_current_frame =
      current_frame_in_part * FRAME_BYTE_SIZE;
  setFrame(decodedFrames + offset_to_current_frame);
  current_frame++;
  current_frame_in_part++;
  if (current_frame >= (nframes + 1)) {
    // `we  are done with all frames
    current_frame = 0;
    current_part = 0;
    return 0;
  } else {
    // are we done with the part ?
    if (current_frame_in_part == frames_in_current_part) {
      current_frame_in_part = 0;
      current_part++;
    }
  }
  return 1;
}
