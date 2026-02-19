#pragma once
#include <ege/runtime.hpp>
#include <ege/engine/render_pipeline.hpp>
#include <functional>
#include <vector>
#include <string>
#include <SDL.h>

namespace ege { namespace ui {

struct MenuLayer : public ege::Layer {
    MenuLayer(int16_t width, int16_t height) : w(width), h(height) {}

    void add_item(const std::string &label, std::function<void()> cb) {
        items.emplace_back(Item{label, cb, {0,0,0,0}});
        compute_layout();
    }

    bool on_event(const ege::Event &e) override {
        if (!visible) return false;
        if (e.type != ege::EventType::Input) return true;

        // Mouse click
        if (e.payload.i == 1 && e.id == 1) { // left mouse button
            int mx = e.pos.x; int my = e.pos.y;
            for (const auto &item : items) {
                const auto &r = item.rect;
                if (mx >= r.x && mx < r.x + r.w && my >= r.y && my < r.y + r.h) {
                    if (item.cb) item.cb();
                    return true;
                }
            }
            return true;
        }

        // Key down
        if (e.payload.i == 1) {
            const uint32_t K_UP = static_cast<uint32_t>(SDLK_UP);
            const uint32_t K_DOWN = static_cast<uint32_t>(SDLK_DOWN);
            const uint32_t K_RETURN = static_cast<uint32_t>(SDLK_RETURN);
            const uint32_t K_KP_ENTER = static_cast<uint32_t>(SDLK_KP_ENTER);
            const uint32_t K_ESCAPE = static_cast<uint32_t>(SDLK_ESCAPE);
            uint32_t id = e.id;
            if (id == K_DOWN) { if (!items.empty()) selected = (selected + 1) % items.size(); return true; }
            if (id == K_UP) { if (!items.empty()) selected = (selected + items.size() - 1) % items.size(); return true; }
            if (id == K_RETURN || id == K_KP_ENTER) { if (selected < items.size() && items[selected].cb) items[selected].cb(); return true; }
            if (id == K_ESCAPE) { hide(); return true; }
        }

        return true; // capture everything while visible
    }

    void on_update(float) override { }

    void on_render(int) override {
        cmdbuf_->push_rect(0, 0x80000000u, 0, 0, w, h);
        // draw buttons
        for (const auto &item : items) {
            const auto &r = item.rect;
            uint32_t color = (&item == &items[selected]) ? 0xFFFFAA00u : 0xFFC0C0C0u;
            cmdbuf_->push_rect(0, color, r.x, r.y, r.w, r.h);
        }
    }

private:
    struct Item { std::string label; std::function<void()> cb; struct Rect{int16_t x,y,w,h;} rect; };
    std::vector<Item> items;
    int16_t w = 320, h = 240;
    size_t selected = 0;
    bool visible = true;

    void compute_layout() {
        const int16_t bw = std::min<int16_t>(220, w - 40);
        const int16_t bh = 30;
        const int16_t spacing = 8;
        if (items.empty()) return;
        int16_t total_h = static_cast<int16_t>(static_cast<int16_t>(items.size()) * static_cast<int16_t>(bh + spacing) - spacing);
        int16_t start_y = static_cast<int16_t>((h - total_h) / 2);
        int16_t x = static_cast<int16_t>((w - bw) / 2);
        for (size_t i = 0; i < items.size(); ++i) {
            items[i].rect = { x, static_cast<int16_t>(start_y + static_cast<int16_t>(i) * (bh + spacing)), bw, bh };
        }
    }
};

}} // namespace ege::ui
