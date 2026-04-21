#pragma once
#include <functional>
#include <M5Unified.h>
#include "../core/AppBase.h"

enum class MenuItemType { APP, ACTION };

struct MenuItem {
    const char*                                              label;
    MenuItemType                                             type;
    std::function<AppBase*()>                                appFactory;
    std::function<void()>                                    action;
    // Icon draw function: (canvas, centerX, centerY, size, color)
    std::function<void(M5Canvas&, int, int, int, uint32_t)>  iconFn;
};
