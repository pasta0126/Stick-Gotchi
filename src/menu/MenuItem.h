#pragma once
#include <functional>
#include <vector>
#include <M5Unified.h>
#include "../core/AppBase.h"

enum class MenuItemType { APP, ACTION, SUBMENU };

struct MenuItem {
    const char*                                              label;
    MenuItemType                                             type;
    std::function<AppBase*()>                                appFactory;  // APP
    std::function<void()>                                    action;      // ACTION
    std::function<void(M5Canvas&, int, int, int, uint32_t)>  iconFn;
    std::vector<MenuItem>                                    children;    // SUBMENU
};
