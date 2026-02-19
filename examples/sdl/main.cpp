#include <chrono>
#include <thread>
#include <iostream>

#include <ege/backends/sdl/sdl_backend.hpp>
#include <ege/engine/render_pipeline.hpp>
#include <ege/engine/render_command.hpp>
#include <ege/runtime.hpp>

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
            if (e.type == ege::EventType::Input && e.id == uint32_t(ege::InputCode::Quit)) return true;
            return false;
        }
        void on_update(float dt) override { (void)dt; ++frame; }
        void on_render(ege::SPSCRenderPipeline<1024,4,8>& pipeline, int /*fc*/) override {
            auto &buf = pipeline.begin_frame();
            buf.push_clear(0xFF001144);
            int x = 10 + (frame % 100);
            buf.push_rect(0, 0xFFFFAA00,
                         static_cast<int16_t>(x),
                         static_cast<int16_t>(40),
                         static_cast<int16_t>(50),
                         static_cast<int16_t>(30));
            pipeline.submit_frame();
        }
    };

    ege::PhysicsSystem physics;
    ege::Runtime rt(backend, pipeline, physics);
    ExampleLayer layer;
    rt.push_layer(&layer);
    rt.run();

    backend.shutdown();
    return 0;
}