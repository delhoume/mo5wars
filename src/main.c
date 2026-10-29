/*
 * Frederic Delhoume 2026
 *
 */

#include "common.h"
#include "delays.h"
#include "filereader.h"
#include "nitram5x5_mini.h"

#define PRC ((unsigned char *)0xA7C0) // VRAM bank select (bit0: olor, 1=form)
#define VIDEO_REG ((unsigned char *)0xA7E7) // Video mode register
#define VRAM ((unsigned char *)0x0000)      // Video memory base address
#define VBL_BIT 0x80
#define VBL_REG ((unsigned char *)0xA7E7)

void my_wait_vbl(void) {
  while (*VBL_REG & VBL_BIT)
    ;
  while (!(*VBL_REG & VBL_BIT))
    ;
}

#define C_BLACK 0
#define C_RED 1
#define C_GREEN 2
#define C_YELLOW 3
#define C_BLUE 4
#define C_MAGENTA 5
#define C_CYAN 6
#define C_WHITE 7
#define C_GRAY 8
#define C_LIGHT_RED 9
#define C_LIGHT_GREEN 10
#define C_LIGHT_YELLOW 11
#define C_LIGHT_BLUE 12
#define C_PURPLE 13
#define C_LIGHT_CYAN 14
#define C_ORANGE 15

#define COLOR(bg, fg) (unsigned char)(((bg) & 0x0F) | (((fg) & 0x0F) << 4))
unsigned char *DRAW_TARGET = VRAM;
unsigned int DRAW_TARGET_PIXEL_HEIGHT = SCREEN_PIXEL_HEIGHT;
unsigned int DRAW_TARGET_BYTE_WIDTH = SCREEN_WIDTH_BYTES;
unsigned int DRAW_TARGET_PIXEL_WIDTH = SCREEN_PIXEL_WIDTH;
// precomputed values for the targets
unsigned int DRAW_TARGET_BYTE_WIDTH2 = DRAW_TARGET_BYTE_WIDTH * 2;
unsigned int DRAW_TARGET_BYTE_WIDTH3 = DRAW_TARGET_BYTE_WIDTH * 3;
unsigned int DRAW_TARGET_BYTE_WIDTH4 = DRAW_TARGET_BYTE_WIDTH * 4;

#define FOREGROUND 0xff
#define BACKGROUND 0x00

unsigned int show_info = 1;
unsigned int show_adv_info = 0;
unsigned int frame = 0;
unsigned int tframes= 0;

#define DRAW_CHAR drawCharFast

// terminal drawing functions are in local draw target coordinates
void drawPoint(int x, int y) {
  // always in bounds, no checks for speed.
  // if (x < 0 || x >= DRAW_TARGET_PIXEL_WIDTH || y < 0 ||
  //     y >= DRAW_TARGET_PIXEL_HEIGHT)
  //   return;
  int xstart = x / 8;
  int xoffset = x % 8;
  unsigned char *dst = DRAW_TARGET + y * DRAW_TARGET_BYTE_WIDTH + xstart;
  *dst |= ((unsigned char)0x01 << (7 - xoffset));
}

static unsigned char bitmask[8] = {0x80, 0x40, 0x20, 0x10,
                                   0x08, 0x04, 0x02, 0x01};
void drawCharSlow(int x, int y, int c) {
  if (c == ' ')
    return;
  unsigned char *src = nitramfont5 + c * GHEIGHT;
  for (int row = 0; row < GHEIGHT; row++, src++) {
    unsigned char charline = *src;
    for (int col = 0; col < GWIDTH; col++) {
      if ((charline & (bitmask[col])) != 0) {
        drawPoint(x + col, y + row);
      }
    }
  }
}
// unsigned char font_A[] = {0x38, 0x6C, 0x7C, 0x6C, 0x6C};

void drawCharFast(int x, int y, int c) {
  if (c == ' ')
    return;
  unsigned int byte_x = x >> 3;
  unsigned char shift = (unsigned char)(x & 7);

  unsigned char *dest =
      DRAW_TARGET + ((unsigned int)y * DRAW_TARGET_BYTE_WIDTH) + byte_x;
  unsigned char *char_ptr = nitramfont5 + c * GHEIGHT;
  unsigned char row_data;

  // Row 1
  row_data = *char_ptr++;
  *dest |= (row_data >> shift); // OR for the left side
  if (shift > 0) {
    *(dest + 1) |= (row_data << (8 - shift)); // OR for the right side
  }
  dest += DRAW_TARGET_BYTE_WIDTH;

  // Row 2
  row_data = *char_ptr++;
  *dest |= (row_data >> shift);
  if (shift > 0) {
    *(dest + 1) |= (row_data << (8 - shift));
  }
  dest += DRAW_TARGET_BYTE_WIDTH;

  // Row 3
  row_data = *char_ptr++;
  *dest |= (row_data >> shift);
  if (shift > 0) {
    *(dest + 1) |= (row_data << (8 - shift));
  }
  dest += DRAW_TARGET_BYTE_WIDTH;

  // Row 4
  row_data = *char_ptr++;
  *dest |= (row_data >> shift);
  if (shift > 0) {
    *(dest + 1) |= (row_data << (8 - shift));
  }
  dest += DRAW_TARGET_BYTE_WIDTH;

  // Row 5
  row_data = *char_ptr;
  *dest |= (row_data >> shift);
  if (shift > 0) {
    *(dest + 1) |= (row_data << (8 - shift));
  }
}

//  Reverses the bits of a single byte (MSB <-> LSB)
unsigned char reverse_byte_bits(unsigned char b) {
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}

// all drawing is arre in draw target coordinate
// no checksfor speed
int fillBand(unsigned char value, int y, unsigned int height) {
  int i = 0;
  if (height == 0)
    return y;

  for (; i < height; i++) {
    if (y + i >= DRAW_TARGET_PIXEL_HEIGHT)
      break;
    unsigned char *dst = DRAW_TARGET + (y + i) * DRAW_TARGET_BYTE_WIDTH;
    memset(dst, value, DRAW_TARGET_BYTE_WIDTH);
  }
  return y + i;
}

void clearTarget() {
  memset(DRAW_TARGET, BACKGROUND,
         DRAW_TARGET_BYTE_WIDTH * DRAW_TARGET_PIXEL_HEIGHT);
}

int drawString(const unsigned char *str, int x, int y) {
  int sx = x;
  unsigned char c;
  while ((c = *str++) != 0) {
    DRAW_CHAR(x, y, c);
    x += GWIDTH + 1;
  }
  return x - sx;
}

// deliberate slow drawing for special effect
int drawStringSlow(const unsigned char *str, int x, int y) {
  int sx = x;
  unsigned char c;
  while ((c = *str++) != 0) {
    drawCharSlow(x, y, c);
    x += GWIDTH + 1;
  }
  return x - sx;
}

void SetGenericTarget(char *buffer, int pixelheight, int widthbytes) {
  DRAW_TARGET = buffer;
  DRAW_TARGET_PIXEL_HEIGHT = pixelheight;
  DRAW_TARGET_BYTE_WIDTH = widthbytes;
  DRAW_TARGET_PIXEL_WIDTH = widthbytes * 8;
  // precompute any necessary values for the target here
  DRAW_TARGET_BYTE_WIDTH2 = DRAW_TARGET_BYTE_WIDTH * 2;
  DRAW_TARGET_BYTE_WIDTH3 = DRAW_TARGET_BYTE_WIDTH * 3;
  DRAW_TARGET_BYTE_WIDTH4 = DRAW_TARGET_BYTE_WIDTH * 4;
}

void set_vram_target() {
  SetGenericTarget(VRAM, SCREEN_PIXEL_HEIGHT, SCREEN_WIDTH_BYTES);
}

#define DOUBLE_BUFFER_SIZE (FRAME_PIXEL_HEIGHT * SCREEN_WIDTH_BYTES)
unsigned char FRAME_BUFFER[DOUBLE_BUFFER_SIZE];

void setFrameTarget() {
  SetGenericTarget(FRAME_BUFFER, FRAME_PIXEL_HEIGHT, SCREEN_WIDTH_BYTES);
}

void commitFrameTarget() {
  unsigned char *dst = VRAM + (FRAME_START_Y * SCREEN_WIDTH_BYTES);
  unsigned char *src = FRAME_BUFFER;
  unsigned int size = FRAME_PIXEL_HEIGHT * SCREEN_WIDTH_BYTES;
  memcpy(dst, src, size);
}

// not valid when we have a string with newlines
int drawStringCenteredH(const unsigned char *str, int y) {
  int len = strlen((const char *)str);
  int x = (SCREEN_PIXEL_WIDTH - ((GWIDTH + 1) * len)) / 2;
  return drawStringSlow(str, x, y);
}

void set_color_mode() { *PRC &= ~0x01; }
void set_form_mode() { *PRC |= 0x01; }

#define LINE_BUFFER_SIZE (GHEIGHT * SCREEN_WIDTH_BYTES)
unsigned char LINE_BUFFER[LINE_BUFFER_SIZE];

void setTextTarget() {
  SetGenericTarget(LINE_BUFFER, GHEIGHT, SCREEN_WIDTH_BYTES);
}

// full width buffer
// 
void commitTextTarget(int y) {
    memcpy(VRAM + y * SCREEN_WIDTH_BYTES, LINE_BUFFER,
           SCREEN_WIDTH_BYTES * GHEIGHT);
}

char sbuffer[64] = "";
static unsigned char readerInitFailed = 0;
int drawFdInfo(int x, int y) {
  unsigned char currentBlock = 0;
  unsigned char currentSector = 0;
  GetCurrentFileInfo(&currentBlock, &currentSector);
  sprintf(sbuffer, "Bl %d Se %d", currentBlock, currentSector);
  return drawString((const unsigned char *)sbuffer, x, y);
}

int drawFdInfoDebug(int x, int y) {
  unsigned char readSuccesses = 0;
  unsigned char readErrors = 0;
  unsigned char lastReadError = 0;
  GetFileReadStatus(&readSuccesses, &readErrors, &lastReadError);
  sprintf(sbuffer, "Ok %d Err %d Last err %d", readSuccesses, readErrors,
          lastReadError);
  return drawString((const unsigned char *)sbuffer, x, y);
}

int drawFrameInfo(int x, int y) {
  sprintf(sbuffer, "%4d/%d", frame + 1, tframes);
  return drawString((const unsigned char *)sbuffer, x, y);
}

int drawInfo(int x, int y) {
  if (show_info || show_adv_info) {
    setTextTarget();
    clearTarget();
    if (show_info)
      x += drawFrameInfo(x, y) + WORD_OFFSET;
    if (show_adv_info) {
      x += drawFdInfo(x, y) + WORD_OFFSET;
      x += drawFdInfoDebug(x, y) + WORD_OFFSET;
    }
  }
  return x;
}

unsigned char *cframe = 0;

void drawFrame() {
  setFrameTarget();
  clearTarget();
  for (unsigned int y = 0; y < FRAME_CHAR_HEIGHT; y++) {
    register unsigned char *frame_ptr = cframe + y * FRAME_CHAR_WIDTH;
    for (unsigned int x = 0; x < FRAME_CHAR_WIDTH; x++) {
      unsigned char c = frame_ptr[x];
      DRAW_CHAR(x * TWIDTH, y * THEIGHT, c);
    }
  }
}

void setFrame(unsigned char *frame) { cframe = frame; }

Reader *reader = 0;

void init_all() {
  *PRC = 0x00;
  *VIDEO_REG |= 0x01;

  // mute_beep
  *((unsigned char *)0xA7C1) = 0x00;
  unsigned int i;

  unsigned char mask = (unsigned char)((1 << GWIDTH) - 1);
  for (i = 0; i < sizeof(nitramfont5); i++) {
    nitramfont5[i] = reverse_byte_bits(nitramfont5[i] & mask);
  }
  set_vram_target();
  set_form_mode();
  clearTarget();
  set_color_mode();
  fillBand(COLOR(C_BLACK, C_GREEN), 0, DRAW_TARGET_PIXEL_HEIGHT);
  fillBand(COLOR(C_BLACK, C_YELLOW), 10, GHEIGHT);
  fillBand(COLOR(C_BLACK, C_WHITE), BAND_PIXEL_HEIGHT, FRAME_PIXEL_HEIGHT);

  //  fixed contents
  // we are only changing form from here
  set_form_mode();

  drawStringCenteredH((const unsigned char *)"THOMSON MO5 ASCII Wars",
                      (BAND_PIXEL_HEIGHT - GHEIGHT) / 2);

  unsigned int start_info_y =
      SCREEN_PIXEL_HEIGHT - ((BAND_PIXEL_HEIGHT - 2 * THEIGHT) / 2);
  drawStringCenteredH(
      (const unsigned char
           *)"Animation: Simon Jensen | Code: Frederic Delhoume",
      start_info_y);
  start_info_y += THEIGHT;
  drawStringCenteredH(
      (const unsigned char *)"www.asciimation.co.nz | github.com/delhoume  ",
      start_info_y);

  reader = GetFileReader(ANIMATION_FILE, MODE_8BIT);

  if (reader == 0) {
    readerInitFailed = 1;
    return;
  }
  SetReader(reader);
  reader->reset();
}

void reboot() {
  // Create a function pointer targeting the MO5 Monitor entry point
  void (*reset_vector)(void) = (void (*)(void))0xF000;

  // Execute the jump (calls the initialization routine)
  reset_vector();
}

void delay_ms(unsigned int ms) {
// Fonction d'attente calibrée en millisecondes
for (unsigned int i = 0; i < 50; i++) {
  asm {
        LDX ms // Load the millisecond count into register X    
    ms_loop:
          // --- Inner Loop: Burns eelow takes 6 clock cycles on a 6809.
          // 166 iterations * 6 xactly 1,000 cycles (1 millisecond) ---
          // Each loop iteration bcycles = 996 cycles.
    // Adding the overhead setup gives exactly 1,000 cycles.
        LDB #166 // [2 cycles]
    burn_cycles:
        DECB // [2 cycles]
        BNE burn_cycles // [3 cycles when branching, 2 when falling
                // --- End of inner loop ---

        LEAX -1,X // [5 cycles] Decrement our millisecond counter
        BNE ms_loop // [3 cycles] Loop until X hits 0
  }
}
}

void showFatalError(const unsigned char *message) {
  drawStringCenteredH(message, (SCREEN_PIXEL_HEIGHT - GHEIGHT) / 2);
  while (1) {
    my_wait_vbl();
  }
}

int main() {
  // CMOC's default stack sits in the monitor's page $20 variables
  // (DK block at $2048), so move it to the top of user RAM.
  asm {
    LDS #$9F00
  }
  init_all();
  if (readerInitFailed) {
    sprintf(sbuffer, "%s initialization error", ANIMATION_FILE);
    showFatalError((const unsigned char *)sbuffer);
  }
  while (1) {
    if (ZXGetNextFrame() == 0) {
      if (FileReaderHasError()) {
        sprintf(sbuffer, "%s error", ANIMATION_FILE);
        showFatalError((const unsigned char *)sbuffer);
      }
      reboot();
    }
    unsigned int foo= 0;
    GetFrameInfo(&foo, &tframes);
    if (show_info || show_adv_info) {
      drawInfo(5, 0);
    }
    drawFrame(cframe);
    if (show_info || show_adv_info) {
      commitTextTarget(10);
    }
    // my_wait_vbl();
    commitFrameTarget();
    // if (touche_F())     how_info = 1 - show_inf;
    // if (touche_D())      show_adv_info = 1 - show_adv_info;
    delay_ms((unsigned int)(delays[frame]));
    frame++;
  }
  return 0;
}
