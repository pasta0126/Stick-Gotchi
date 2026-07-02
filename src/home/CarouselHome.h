#pragma once
#include <vector>
#include <functional>
#include "../core/AppBase.h"
#include "../core/DisplayManager.h"

class AppManager;

// A tile in the home carousel: icon + label + factory for the app it launches.
// iconFn(canvas, cx, cy, pixelScale, selected) — selected picks a bright vs dim palette.
struct CarouselTile {
    const char*                                           name;
    std::function<void(M5Canvas&, int, int, int, bool)>   iconFn;
    std::function<AppBase*()>                             launch;
};

// Root launcher screen. Horizontal, circular carousel of tiles.
// BtnB short = advance, BtnA short = launch selected tile.
// BtnB long is a no-op here — Home has no level above it.
class CarouselHome : public AppBase {
public:
    void inject(DisplayManager* display) { _display = display; }
    void setAppManager(AppManager* apps)  { _apps = apps; }
    void addTile(const CarouselTile& tile) { _tiles.push_back(tile); }

    void init()                       override;
    void update(uint32_t deltaMs)     override;
    void suspend()                    override {}
    void resume()                     override {}
    void destroy()                    override {}
    bool onInput(const InputEvent& e) override;
    const char* getName() const       override { return "Home"; }

private:
    DisplayManager*           _display = nullptr;
    AppManager*               _apps    = nullptr;
    std::vector<CarouselTile> _tiles;
    size_t                    _index   = 0;   // persists within a power-on session
    bool                      _dirty   = true;

    void _draw();
};
