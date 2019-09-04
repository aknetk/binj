#ifndef SUBTITLEPACKET_H
#define SUBTITLEPACKET_H
struct SubtitlePacket {
    double pts_start;
    double pts_end;
    int x;
    int y;
    bool clear;
    SDL_Surface* surface;
};
#endif /* SUBTITLEPACKET_H */
