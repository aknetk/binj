#ifndef COLOR_H
#define COLOR_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


#include <Udon/Standard.h>

class Color {
public:
    Uint8 A = 0x00;
    Uint8 R = 0x00;
    Uint8 G = 0x00;
    Uint8 B = 0x00;

    static Color FromARGB(int alpha, int red, int green, int blue);
    static Color FromARGB(int red, int green, int blue);
    static Color FromARGB(int alpha, Color baseColor);
    static Color FromARGB(int alpha, int hex);
    static Color FromARGB(int argb);
    static Color FromHex(const char* hexString);
    static Color FromHex(char* hexString);
    static Color BlendColor(Uint32 color1, Uint32 color2, double percent);
    static Color FromHSL(double hue, double saturation, double lightness);
    static Color FromHSLA(double hue, double saturation, double lightness, double alpha);
};

#endif /* COLOR_H */
