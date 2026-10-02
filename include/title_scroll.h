#ifndef BETTER_FAVORITES_TITLE_SCROLL_H
#define BETTER_FAVORITES_TITLE_SCROLL_H
#include <cstdint>
#include <string>

// Pixel-only animation state. Never slices UTF-8 or changes browser selection.
class TitleScroll {
public:
    static constexpr unsigned delayMs = 1000, endHoldMs = 1000, pixelsPerSecond = 30;
    int update(const std::string& identity, const std::string& label,
               int textWidth, int visibleWidth, std::uint32_t now, bool paused);
    void restart(std::uint32_t now);
    void pause(std::uint32_t now);
private:
    std::string identity_, label_;
    int textWidth_ = 0, visibleWidth_ = 0;
    bool initialized_ = false, paused_ = false;
    std::uint32_t lastTick_ = 0;
    std::uint64_t elapsed_ = 0;
};
#endif
