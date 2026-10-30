This is Simon Jensen https://www.asciimation.co.nz/ ported to 80's French 16 bit micro computer Thomson MO5.

1 Mhz 6809, 48k memory with 320x200 video output.

Result is a 360k bootable disk image output/mo5wars.fd that can  be loaded in emulators :
- Yame
- DCmoto

All 3845 frames are displayed, but the 320 pixels horizontal resolution only allows 64 chars with a 5x5 font,
so the animation is cropped to 64x13.

Video dedicated memory takes 8k out of the 48k. so memory constraints are high.
These 8k are shared between "form" and "color"  banks.
 
 - "color" defines pixel color. each byte is divided in 2 nibbles,
one for the background and one for the foreground that give an index to a color in a 4 bit  (15 colors) palette.

- "form" describes a pixel color with 1 bit, 0 for background and 1 for foreground

1) preparing the animation

- each line is converted from variable up to 67 chars by padding with spaces, with delay lines removed
- 1 char is trimmed on the left and 2 on the right
- with space padding we have in fact a better compression than leaving end of line.
- raw 13x64x3845 are grouped into 10 frames parts, 385 parts of 13x64x10 = 8320 bytes
- each part is zx0 compressed to a size from 8320 to 25 to 777 bytes, a very large compression.
- groups of 10 frames leads to less compression but make streaming easier, as 8320 by can be allocated in memory to hold 10 uncompressed frames.
- zx0 compressed parts are packed in a single 124641 byte file with minimal additional structure
- delays between frames are handled separately and an .h include file is generated from it.

