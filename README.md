This is Simon Jensen https://www.asciimation.co.nz/ ported to 80's French 16 bit micro computer Thomson MO5.

1 Mhz 6809, 48k memory with 320x200 video output.

Result is a 360k bootable disk image output/mo5wars.fd that can  be loaded in emulators :
- Yame
- DCmoto

All 3845 frames are displayed, but the 320 pixels horizontal resolution only allows 64 chars with a 5x5 font,
so the animation is cropped to 64x13.

Video dedicated memory takes 8k out of the 48k. so memory constraints are high.
These 8k are shared between "form" and "color" banks, noe of them is accessible at a given moment.
 
 - "color" defines pixel color. each byte is divided in 2 nibbles,
one for the background and one for the foreground that give an index to a color in a 4 bit  (15 colors) palette.

- "form" describes a pixel color with 1 bit, 0 for background and 1 for foreground
- 
1) preparing the animation

- each line is converted from variable length terminayted by '\n' up to 67 chars by padding with spaces, with delay lines removed
- 1 char is trimmed on the left and 2 on the right to, match 320 pixels as 64 5 pixel wide font.
- with space padding we have in fact a better compression than leaving end of line.
- raw 13x64x3845 are grouped into 10 frames parts, 385 parts of 13x64x10 = 8320 bytes
- each part is zx0 compressed from 8320 to 25 - 777 bytes, a very large compression ratio
- groups of 10 frames leads to less compression but make streaming easier, as 8320 by can be allocated in memory to hold 10 uncompressed frames.
- zx0 compressed parts are packed in a single 124641 byte file with minimal additional structure
- delays between frames are handled separately and an .h include file is generated from it.

2) rendering the animation

- in a loop a 10 frame bloc is loaded into memory  (max 777 byte buffer) and uncompressed to a 8320 byte buffer
- the ascii buffer isndividd in 10 frames, each is rendered in sequence to a 320 pixel x 13 lines (5 pixel font + 1 pixel between lines) "form" double buffer
- double buffer is copied in video ram on vertical sync signal, if rendering was made directly to video memory, it would take several vbl, and partial frames would be displayed, resulting in flickering. My estimation is that max 100 chars can be rendered between two vertical blank.
- we wait for the frame defined delay milliseconds
- the next 9 frames are rendered without decompression or disk access.


3) Performance
 - the mo5 is very slow, anm code is written in C, moderately optimised.
 - the actual complete animation rendering takes about 1h15  (now 1h5 with char drawing optimisation)
 - Each frame is fully rendered from the uncompressed 64x13 ascii buffer
 - The critical path is in individual char rendering but a number of other optimisations could be very effective:
   - keeping a previous frame and drawing only differences to the double buffer (simple)
   - rendering directly the zx0 opcodes to graphics, leveraging sequences of similar chars
   - as font is 5x5 rendering each char individually involves heavy bitwise operations depending on the x location of the char, we can use full byte copy for inner repetitions when sequence of the same char is drawn (this is the most promising possible gain)
   - the cost of decompression could be distributed per frame instead of per 10 frames part.
   - The cost of reading a new sector from the disk each 256 bytes means have a  noticeable additional delay each variable number of frames depending on compression
   - rewriting part in assembly or reusing assembly versions of zx0 could bring some gain.
   - getting rid of fat format and accessing the drive directly as raw sectors could be good for performance.
   - digging into asciinema for ideas
