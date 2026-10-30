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
one for the background and one for the foreground ther gve an index to a color in a 4 bit  (15 colors) palette.

- "form" describes a pixel color with 1 bit, 0 for background and 1 for foreground

1) preparing the animation

- each line is converted from variable up to 67 chars by padding with spaces

