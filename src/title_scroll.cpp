#include "title_scroll.h"
#include <algorithm>

void TitleScroll::restart(std::uint32_t now) {
    elapsed_ = 0;
    lastTick_ = now;
}

void TitleScroll::pause(std::uint32_t now) {
    if (initialized_ && !paused_) elapsed_ += std::uint32_t(now-lastTick_);
    paused_ = true;
    lastTick_ = now;
}

int TitleScroll::update(const std::string& identity, const std::string& label,
                        int textWidth, int visibleWidth, std::uint32_t now, bool paused) {
    if (!initialized_ || identity != identity_ || label != label_ ||
        textWidth != textWidth_ || visibleWidth != visibleWidth_) {
        identity_ = identity;
        label_ = label;
        textWidth_ = textWidth;
        visibleWidth_ = visibleWidth;
        initialized_ = true;
        restart(now);
    } else if (!paused_) {
        // Unsigned subtraction handles SDL_GetTicks' 32-bit wrap at 49 days.
        elapsed_ += std::uint32_t(now - lastTick_);
    }
    if (paused_ && !paused) restart(now);
    paused_ = paused;
    lastTick_ = now;
    if (visibleWidth <= 0 || textWidth <= visibleWidth) return 0;
    const auto overflow = std::uint64_t(textWidth - visibleWidth);
    const auto travel = (overflow * 1000 + pixelsPerSecond - 1) / pixelsPerSecond;
    const auto phase = elapsed_ % (delayMs + travel + endHoldMs);
    if (phase < delayMs) return 0;
    return int(std::min(overflow, (phase - delayMs) * pixelsPerSecond / 1000));
}
