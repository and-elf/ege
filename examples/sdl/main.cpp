#include <chrono>
#include <thread>
#include <iostream>
#include <print>
#include <ege/backends/sdl/sdl_backend.hpp>
#include <ege/engine/render_pipeline.hpp>
#include <ege/engine/render_command.hpp>
#include <ege/runtime.hpp>
#include "ui.hpp"

int main() {
    ege::backend::SDLBackend backend;
    const std::size_t w = 320, h = 240;
    if (!backend.init(w, h)) return 1;

    using Pipeline = ege::SPSCRenderPipeline<1024, 4, 8>;
    Pipeline pipeline;

    // Simple example layer
    struct ExampleLayer : public ege::Layer {
        int frame = 0;

        bool on_event(const ege::Event &e) override {
            if(e.is_right_click()) {
                std::print("Right click at ({} {})", e.pos.x, e.pos.y);
                return true; // consume right-click events
            }
            return false;
        }
        void on_update(float dt) override { (void)dt; ++frame; }
        void on_render(int /*fc*/) override {
            cmdbuf_->push_clear(0xFF001144);
            int x = 10 + (frame % 100);
            cmdbuf_->push_rect(0, 0xFFFFAA00,
                         static_cast<int16_t>(x),
                         static_cast<int16_t>(40),
                         static_cast<int16_t>(50),
                         static_cast<int16_t>(30));
        }
    };

    ege::PhysicsSystem physics;
    ege::Runtime rt(backend, pipeline, physics);
    ExampleLayer layer;
    // wire the menu pointer so the example stops producing frames while the menu is visible
    // create a simple menu layer on top
    ege::ui::MenuLayer menu(static_cast<int>(w), static_cast<int>(h));
    menu.add_item("Resume", [&menu]() { menu.hide(); });
    menu.add_item("Quit", [&rt]() { rt.stop(); });
    menu.show();
    layer.show();
    rt.push_layer(&layer);
    rt.push_layer(&menu);
    rt.run();

    backend.shutdown();
    return 0;
}