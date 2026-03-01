#pragma once
#ifndef UNITY_BUILD
#define IMPLEMENT_ALL   1
#include "main.h"
#endif

// > SUBMODULE: WINDOW

// > INDEX
GUI_WindowTemp GUI_MakeWindowTemp();

// > IMPLEMENTATION
#ifdef IMPLEMENT_ALL
GUI_WindowTemp GUI_MakeWindowTemp()
{
        GUI_WindowTemp result = {
            .current_action     = EGUI_WindowAction_None,
            .control_focus_ptr  = NULL,
            .window_target_id   = 0
        };
    return result;
}
#endif
