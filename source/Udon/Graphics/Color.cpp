#if INTERFACE
#include <Udon/Standard.h>

class Color {
public:
    Uint8 A = 0x00;
    Uint8 R = 0x00;
    Uint8 G = 0x00;
    Uint8 B = 0x00;
};
#endif

#include <Udon/Graphics/Color.h>

#include <Udon/Logging/Log.h>

PUBLIC STATIC Color Color::FromARGB(int alpha, int red, int green, int blue) {
    Color color;
    color.A = alpha & 0xFF;
    color.R = red & 0xFF;
    color.G = green & 0xFF;
    color.B = blue & 0xFF;
    return color;
}
PUBLIC STATIC Color Color::FromARGB(int red, int green, int blue) {
    return Color::FromARGB(0xFF, red, green, blue);
}
PUBLIC STATIC Color Color::FromARGB(int alpha, Color baseColor) {
    return Color::FromARGB(alpha, baseColor.R, baseColor.G, baseColor.B);
}
PUBLIC STATIC Color Color::FromARGB(int alpha, int hex) {
    return Color::FromARGB((alpha & 0xFF) << 24 | hex);
}
PUBLIC STATIC Color Color::FromARGB(int argb) {
    return Color::FromARGB(argb >> 24, argb >> 16, argb >> 8, argb);
}

PUBLIC STATIC Color Color::FromHex(const char* hexString) {
    return Color::FromHex((char*)hexString);
}
PUBLIC STATIC Color Color::FromHex(char* hexString) {
    if (*hexString == '#') return Color::FromHex(hexString + 1);

    Color color;
    Uint8* where;
    if (strlen(hexString) == 8) {
        where = &color.A;
        if (*hexString >= 'a' && *hexString <= 'f') *where = (*hexString - 'a') + 0x1A;
        else if (*hexString >= 'A' && *hexString <= 'F') *where = (*hexString - 'A') + 0x1A;
        else if (*hexString >= '0' && *hexString <= '9') *where = (*hexString - '0') + 0x10;
        else Log::Print(Log::LOG_ERROR, "Invalid character in hexString for Color::FromHex(char*).");
        if (++hexString == 0) Log::Print(Log::LOG_ERROR, "Reached end of hexString for Color::FromHex(char*).");
        if (*hexString >= 'a' && *hexString <= 'f') *where |= (*hexString - 'a') + 0xA;
        else if (*hexString >= 'A' && *hexString <= 'F') *where |= (*hexString - 'A') + 0xA;
        else if (*hexString >= '0' && *hexString <= '9') *where |= (*hexString - '0');
        else Log::Print(Log::LOG_ERROR, "Invalid character in hexString for Color::FromHex(char*).");
        if (++hexString == 0) Log::Print(Log::LOG_ERROR, "Reached end of hexString for Color::FromHex(char*).");
    }

    int i = 0;
    Uint8* whereList[4] = { &color.R, &color.G, &color.B, 0 };
    while (whereList[i]) {
        where = whereList[i];
        if (*hexString >= 'a' && *hexString <= 'f') *where = (*hexString - 'a') + 0x1A;
        else if (*hexString >= 'A' && *hexString <= 'F') *where = (*hexString - 'A') + 0x1A;
        else if (*hexString >= '0' && *hexString <= '9') *where = (*hexString - '0') + 0x10;
        else Log::Print(Log::LOG_ERROR, "Invalid character in hexString for Color::FromHex(char*).");
        if (++hexString == 0) Log::Print(Log::LOG_ERROR, "Reached end of hexString for Color::FromHex(char*).");
        if (*hexString >= 'a' && *hexString <= 'f') *where |= (*hexString - 'a') + 0xA;
        else if (*hexString >= 'A' && *hexString <= 'F') *where |= (*hexString - 'A') + 0xA;
        else if (*hexString >= '0' && *hexString <= '9') *where |= (*hexString - '0');
        else Log::Print(Log::LOG_ERROR, "Invalid character in hexString for Color::FromHex(char*).");
        if (++hexString == 0) Log::Print(Log::LOG_ERROR, "Reached end of hexString for Color::FromHex(char*).");
        i++;
    }

    return color;
}

PUBLIC STATIC Color Color::BlendColor(Uint32 color1, Uint32 color2, double percent) {
    Color color;
    double inv = 1.0 - percent;

    color.A = 0xFF;
    color.R = int((color1 >> 16 & 0xFF) * inv + (color2 >> 16 & 0xFF) * percent);
    color.G = int((color1 >> 8 & 0xFF) * inv + (color2 >> 8 & 0xFF) * percent);
    color.B = int((color1 & 0xFF) * inv + (color2 & 0xFF) * percent);
    return color;
}

PUBLIC STATIC Color Color::FromHSL(double hue, double saturation, double lightness) {
    return Color::FromHSLA(hue, saturation, lightness, 1.0);
}
PUBLIC STATIC Color Color::FromHSLA(double hue, double saturation, double lightness, double alpha) {
    hue = fmod(fmod(hue, 360.0) + 360.0, 360.0);

    int hueI = int(hue / 60.0);
    double C = (1 - abs(2 * lightness - 1)) * saturation;
    double X = C * (1 - abs(fmod(hue / 60.0, 2.0) - 1));
    double M = lightness - C / 2;

    double r, g, b;
    switch (hueI) {
        case 0 : r = C  ; g = X  ; b = 0  ; break; // R to Y
        case 1 : r = X  ; g = C  ; b = 0  ; break; // Y to G
        case 2 : r = 0  ; g = C  ; b = X  ; break; // G to C
        case 3 : r = 0  ; g = X  ; b = C  ; break; // C to B
        case 4 : r = X  ; g = 0  ; b = C  ; break; // B to M
        default: r = C  ; g = 0  ; b = X  ; break; // M to R
    }
    return Color::FromARGB(int(alpha * 0xFF), int((r + M) * 0xFF), int((g + M) * 0xFF), int((b + M) * 0xFF));
}
