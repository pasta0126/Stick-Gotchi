#pragma once
#include <functional>
#include "../core/AppBase.h"

enum class MenuItemType { APP, ACTION };

struct MenuItem {
    const char*              label;
    MenuItemType             type;
    std::function<AppBase*()> appFactory; // Used when type == APP
    std::function<void()>    action;      // Used when type == ACTION
};
