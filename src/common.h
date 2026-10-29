#if !defined(COMMON_H)
#define COMMON_H

#include "filereader.h"
#include <cmoc.h>

#define SCREEN_PIXEL_WIDTH 320
#define SCREEN_PIXEL_HEIGHT 200

#define FRAME_CHAR_WIDTH 64
#define PART_TOTAL 8320
#define MAX_COMPRESSED_PART 800
#define ANIMATION_FILE "ANIM64.ZXS"
#define FRAME_CHAR_HEIGHT 13

#define GWIDTH 5
#define ADVANCEX 0
#define GHEIGHT 5
#define ADVANCEY 1
#define TWIDTH 5
#define THEIGHT 6

#define WORD_OFFSET 5

#define FRAMES_PER_PART 10

#define FRAME_DURATION 20

static int START_Y = (SCREEN_PIXEL_HEIGHT - FRAME_CHAR_HEIGHT * THEIGHT) / 2;

#define SCREEN_WIDTH_BYTES 40 // 320 pixels / 8 pixels per byte
#define SCREEN_SIZE_BYTES                                                      \
  (SCREEN_WIDTH_BYTES * SCREEN_PIXEL_HEIGHT) // 8000 bytes

#define FRAME_PIXEL_WIDTH (FRAME_CHAR_WIDTH * TWIDTH)
#define FRAME_PIXEL_HEIGHT (FRAME_CHAR_HEIGHT * THEIGHT)
#define BAND_PIXEL_HEIGHT ((SCREEN_PIXEL_HEIGHT - FRAME_PIXEL_HEIGHT) / 2)
#define FRAME_START_Y ((SCREEN_PIXEL_HEIGHT - FRAME_PIXEL_HEIGHT) / 2)
#define FRAME_BYTE_SIZE (FRAME_CHAR_WIDTH * FRAME_CHAR_HEIGHT)

typedef unsigned char uint8_t;
typedef unsigned int uint16_t;

void GetFrameInfo(unsigned int *frame, unsigned int *frames);
void setFrame(unsigned char *frame);
int ZXGetNextFrame();

#endif // COMMON_H