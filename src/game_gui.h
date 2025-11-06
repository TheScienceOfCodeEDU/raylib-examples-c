#ifndef UNITY_BUILD
 #include "gui.h"
 #include "game_structs.h"
#endif

void GUI_TopBar(PLAYER_Actions* actions, Rectangle target)
{
    GUI_Setup *setup = GUI_GetSetup();
    GUI_Icons *icons = GUI_GetIcons();
    GUI_Theme *theme = &setup->theme;
    int buttons = 3;
    float button_w = target.width / buttons;
    float button_h = target.height;

    Rectangle shape = (Rectangle) { target.x, target.y, button_w, button_h };
    GUI_BeginFontType(EGUI_FontType_GUI);
        actions->reset_characters    = GUI_Button(shape, "Reset", &icons->New, theme->red);
        actions->add_character       = GUI_Button(MoveRect(shape, (Vector2) { button_w, 0 }), "Add", &icons->Open, theme->gray);
        actions->toggle_character    = GUI_Button(MoveRect(shape, (Vector2) { button_w * 2, 0 }), "Change", &icons->Error, theme->gray);
    GUI_EndFontType();
}
