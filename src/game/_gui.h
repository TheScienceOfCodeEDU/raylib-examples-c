#pragma once
#ifndef NON_EDITOR_BUILD
#define IMPLEMENT_ALL   1
#include "../common.h"
#endif

//
// INDEX
//
void GUI_ProgramMenu();
void GUI_GameMenu();
void GUI_TopBar();
void WIN_Window(GUI_Window* window);
void WIN_Layouts(GUI_Window* window);
void WIN_CharacterDebug(GUI_Window* window);
void WIN_Settings(GUI_Window* window);
void WIN_Winman(GUI_Window* window);

//
// FUNCTIONS
//
#

void GUI_ProgramMenu()
{
    int x = -1;
    GUI_Icons *icons        = GUI_GetIcons();
    GUI_ThemeColors colors  = GUI_OverlayColors();
    GUI_BeginOverlay(true, true);
        GUI_Button(GUI_GridAt(x,1),       "New",          &icons->New,    colors);
        GUI_Button(GUI_GridAt(x,2),       "Open",         &icons->Open,   colors);
        if (GUI_Button(GUI_GridAt(x,3),   "Quit",         &icons->New,    colors))
            exit(0);
    GUI_EndOverlay(GUI_GridBetween(x, 1, x, 3));
}

void GUI_GameMenu()
{
    GUI_Icons *icons        = GUI_GetIcons();
    GAME_Actions *actions   = &GAME_CTX.temp->player_actions;
    GUI_ThemeColors colors  = GUI_OverlayColors();
    GUI_BeginOverlay(true, true);
        GUI_Button(GUI_GridAt(-1, 1), "Game [X]", &icons->Dog, colors);
        actions->reset_characters    = GUI_Button(GUI_GridAt(-1,1), "Reset",    &icons->New,    colors);
        actions->add_character       = GUI_Button(GUI_GridAt(-1,2), "Add",      &icons->Open,   colors);
        actions->toggle_character    = GUI_Button(GUI_GridAt(-1,3), "Change",   &icons->Error,  colors);
    GUI_EndOverlay(GUI_GridBetween(-1, 1, -1, 3));
}

void GUI_TopBar()
{
    GUI_Icons *icons    = GUI_GetIcons();
    GUI_Theme theme     = GUI_GetTheme();
    Vector2 start       = RectPosition(GUI_GridAt(0,0));
    const int BUTTONS   = 4;
    GUI_SetThemeColors(theme.red);
    GUI_GridForCols(BUTTONS, EGUI_Font_GUI);
    GUI_ButtonMenu(GUI_GridNextX(), "Project",     &icons->None,  theme.red,          GUI_ProgramMenu);
    GUI_ButtonMenu(GUI_GridNextX(), "Game",        &icons->Dog,   theme.abstractica,  GUI_GameMenu);
    GUI_ButtonMenu(GUI_GridNextX(), "Other",       &icons->Dog,   theme.gray,         GUI_GameMenu);

    GUI_Face(start, GUI_GridHeightOrDefault());
    GUI_DrawOverlay();
}

//
// SAMPLE WINDOW
//

// Define your draw window
void WIN_Window(GUI_Window* window)
{
    // Prepare your data
    GAME_WindowState *win_state = GAME_CTX.win_state;

    // Responsive height (if you require it)
    // GUI_WindowUpdateShapeForContent(window);

    GUI_Setup *setup            = GUI_GetSetup();
    GUI_Icons *icons            = GUI_GetIcons();
    GUI_Theme theme             = setup->theme;
    // Keep or modify colors
    GUI_ThemeColors colors      = window->colors;
    // Set your font
    EGUI_Font font     = win_state->font_toggle ? EGUI_Font_GUI: EGUI_Font_Default;

    GUI_SetFont(font);
    // A default layout with 3 columns
    GUI_GridForCols(3, font);

    GUI_GridNextX();
    GUI_ButtonMenu(GUI_GridNextX(), "Game 2",    &icons->Dog,    theme.abstractica, GUI_GameMenu);

    GUI_GridForDuplicate();
    // 1st input (textbox)
    GUI_Text(GUI_GridNextX(), "Text", colors);
    GUI_Input(GUI_GridNextXn(2), win_state->input_contents, (int)sizeof(win_state->input_contents), EGUI_Input_Text, colors);

    // 2nd input for integer
    // TODO@dc: add min, max and parsing
    GUI_GridForDuplicate();
    GUI_Text(GUI_GridNextX(), "Int", colors);
    GUI_Input(GUI_GridNextXn(2), win_state->input_int_contents, (int)sizeof(win_state->input_int_contents), EGUI_Input_Int, colors);

    // 3rd input for float
    GUI_GridForDuplicate();
    GUI_Text(GUI_GridNextX(), "Float", colors);
    GUI_Input(GUI_GridNextXn(2), win_state->input_float_contents, (int)sizeof(win_state->input_float_contents), EGUI_Input_Float, colors);

    // Wallpaper check (checkbox/switch)
    // With a theme.red color
    GUI_GridForDuplicate();
    GUI_Text(GUI_GridNextX(), "Wallpaper",  colors);
    GUI_Check(GUI_GridNextXn(2), &win_state->checkbox_value, "ON", "OFF", setup->theme.red);

    // Font toggler
    GUI_GridForDuplicate();
    GUI_Text(GUI_GridNextX(), "Font",  colors);
    GUI_Check(GUI_GridNextXn(2), &win_state->font_toggle, "GUI", "DEF", colors);
}

void WIN_Layouts(GUI_Window* window)
{
    GUI_Setup* setup = GUI_GetSetup();
    EGUI_Font font = EGUI_Font_Default;
    float default_height = GUI_CalcDefaultHeightScaled(font);

    Rectangle window_workspace = window->workspace;
    GUI_SetFont(EGUI_Font_Default);

    // First block
    GUI_GridForXY(window_workspace.width, default_height, 0);
    GUI_Text(GUI_GridNextY(), "Some sample layouts for imKairos", setup->theme.gray);

    float color_alpha = 0.9f;
    GUI_BeginControlScissor();
        DrawDebugRect(GUI_GridNextY(), ColorAlpha(BROWN, color_alpha));
        DrawDebugRect(GUI_GridNextY(), ColorAlpha(BEIGE, color_alpha));

        // 1/3rd and 2/3rds blocks
        GUI_GridForXY(window_workspace.width / 3, default_height, 0);
        DrawDebugRect(GUI_GridNextX(), ColorAlpha(YELLOW, color_alpha));
        DrawDebugRect(GUI_GridNextXn(2), ColorAlpha(GREEN, color_alpha));

        // Second block
        // 3 horizontals of 1/3 of the available space
        GUI_GridForXY(window_workspace.width / 3, default_height, 0);
        DrawDebugRect(GUI_GridNextX(), ColorAlpha(DARKGRAY, color_alpha));
        DrawDebugRect(GUI_GridNextX(), ColorAlpha(GRAY, color_alpha));
        DrawDebugRect(GUI_GridNextX(), ColorAlpha(LIGHTGRAY, color_alpha));

        // Prepare for a new block with 5 elements per row
        // You can send negative values to use AVAILABLE - YOUR_VALUE
        // Ex:
        // -200 means take all space but at least 200
        GUI_GridForXY(window_workspace.width / 5, -200, default_height);
        DrawDebugRect(GUI_GridNextX(), BLACK);
        DrawDebugRect(GUI_GridNextX(), BLACK);
        DrawDebugRect(GUI_GridNextX(), BLACK);
        DrawDebugRect(GUI_GridNextX(), BLACK);
        DrawDebugRect(GUI_GridNextX(), BLACK);

        // Final row
        GUI_GridForXY(window_workspace.width / 2, default_height, 0);
        DrawDebugRect(GUI_GridNextX(), ColorAlpha(RED, color_alpha));
        DrawDebugRect(GUI_GridNextX(), ColorAlpha(BLUE, color_alpha));
    EndScissorMode();

}


void WIN_CharacterDebug(GUI_Window* window)
{
    GAME_State *game_state      = GAME_CTX.state;
    GAME_Character *ch          = &game_state->characters[game_state->current_character];
    GUI_ThemeColors colors      = window->colors;
    EGUI_Font font     = EGUI_Font_Default;

    GUI_GridForCols(2, font);

    // Shape
    GUI_Text(GUI_GridNextX(), "shape.x", colors);
    GUI_Text(GUI_GridNextX(), TextFormat("%.2f", ch->shape.x), colors);

    GUI_GridForDuplicate();
    GUI_Text(GUI_GridNextX(), "shape.y", colors);
    GUI_Text(GUI_GridNextX(), TextFormat("%.2f", ch->shape.y), colors);

    GUI_GridForDuplicate();
    GUI_Text(GUI_GridNextX(), "shape.w", colors);
    GUI_Text(GUI_GridNextX(), TextFormat("%.2f", ch->shape.width), colors);

    GUI_GridForDuplicate();
    GUI_Text(GUI_GridNextX(), "shape.h", colors);
    GUI_Text(GUI_GridNextX(), TextFormat("%.2f", ch->shape.height), colors);

    // Color (RGB)
    GUI_GridForDuplicate();
    GUI_Text(GUI_GridNextX(), "color", colors);
    GUI_Text(GUI_GridNextX(), TextFormat("r:%d g:%d b:%d a:%d",
        ch->color.r, ch->color.g, ch->color.b, ch->color.a), colors);

    // Movement
    GUI_GridForDuplicate();
    GUI_Text(GUI_GridNextX(), "movement.x", colors);
    GUI_Text(GUI_GridNextX(), TextFormat("%.3f", ch->movement.x), colors);

    GUI_GridForDuplicate();
    GUI_Text(GUI_GridNextX(), "movement.y", colors);
    GUI_Text(GUI_GridNextX(), TextFormat("%.3f", ch->movement.y), colors);

    // Animation time
    GUI_GridForDuplicate();
    GUI_Text(GUI_GridNextX(), "anim_time", colors);
    GUI_Text(GUI_GridNextX(), TextFormat("%.3f", ch->anim_time), colors);
}

// Define your draw window
void WIN_Settings(GUI_Window* window)
{
    GUI_State *state            = GUI_GetState();
    GUI_ThemeColors colors      = window->colors;
    EGUI_Font font     = EGUI_Font_Default;

    GUI_GridForCols(3, font);
    GUI_Text(GUI_GridNextX(), "Scale", colors);
    GUI_Float(GUI_GridNextXn(2), &state->scale, colors, 0.5f, 6.0f);

    GUI_GridForDuplicate();
    GUI_Text(GUI_GridNextX(), "Scale 2", colors);
    GUI_Float(GUI_GridNextXn(2), &state->scale, colors, 0.5f, 6.0f);
}

void WIN_Winman(GUI_Window* window)
{
    GUI_State *state = GUI_GetState();
    GUI_Setup *setup = GUI_GetSetup();
    GUI_Icons *icons = GUI_GetIcons();
    EGUI_Font font = EGUI_Font_Default;
    float default_height = GUI_CalcDefaultHeightScaled(font);

    Rectangle window_workspace = window->workspace;

    GUI_GridForXY(window_workspace.width, default_height, 0);

    static GUI_Window* win_window = NULL;
    if (GUI_Button(GUI_GridNextY(), "Sample window", NULL, window->colors)) {
        int win_id = 2;
        if (win_window == NULL || win_window->id == 0) {
            win_window = GUI_OpenWindow(win_id, "Sample window", setup->theme.abstractica, &icons->Dog, false, WIN_Window);
        }
        GUI_ForceZindex(win_id);
    }
    static GUI_Window* win_layouts = NULL;
    if (GUI_Button(GUI_GridNextY(), "Layouts window", NULL, window->colors)) {
        int win_id = 3;
        if (win_layouts == NULL || win_layouts->id == 0) {
            win_layouts = GUI_OpenWindow(win_id, "Layouts window", setup->theme.gray, &icons->Layouts, false, WIN_Layouts);
        }
        GUI_ForceZindex(win_id);
    }
    static GUI_Window* win_character_debug = NULL;
    if (GUI_Button(GUI_GridNextY(), "Character debug", NULL, window->colors)) {
        int win_id = 4;
        if (win_character_debug == NULL || win_character_debug->id == 0) {
            win_character_debug = GUI_OpenWindow(win_id, "Character debug", setup->theme.gray, &icons->Dog, false, WIN_CharacterDebug);
        }
        GUI_ForceZindex(win_id);
    }
    static GUI_Window* win_settings = NULL;
    if (GUI_Button(GUI_GridNextY(), "Settings", &icons->Setup, window->colors)) {
        int win_id = 5;
        if (win_settings == NULL || win_settings->id == 0) {
            win_settings = GUI_OpenWindow(win_id, "Settings", setup->theme.gray, &icons->Face, true, WIN_Settings);
        }
        GUI_ForceZindex(win_id);
    }


    GUI_Text(GUI_GridNextY(), "--- Opened windows ---", window->colors);
    GUI_GridForDuplicate();
    for (int i = 0; i < GUI_MAX_OPEN_WINS; ++i) {
        GUI_Window* win = &state->window_s[i];
        if (win->id == 0 || window->id == win->id) continue;
        if (GUI_Button(GUI_GridNextY(), TextFormat("%d - %s", win->id, win->title), NULL, window->colors)) {
            GUI_ForceZindex(win->id);
        }
    }

    GUI_Text(GUI_GridNextY(), "--- Global values ---", window->colors);

    // NOTE:
    // GUI_IsPointerOverGui() is not safe to be called by a Window. It requires ALL windows to be processed beforehand.
    // GUI_Text(GUI_NextVertical(), TextFormat("PointerOverGUI: %d", GUI_IsPointerOverGui()),  window->colors);

    GUI_Window *active = GUI_GetWindowByZindex(0);
    if (active != NULL) {
        Rectangle t_shape = GUI_GetWindowTitle(active->shape);
        Rectangle w_shape = active->shape;

        GUI_Text(GUI_GridNextY(), "--- Focused window ---", window->colors);
        GUI_Text(GUI_GridNextY(), TextFormat("ID=%d scroll=%.2f content_height=%.2f", active->id, active->scroll_offset, active->content_height),  window->colors);

        GUI_Text(GUI_GridNextY(), "title_shape", window->colors);
        GUI_Text(GUI_GridNextY(), TextFormat("x=%.2f  y=%.2f w=%.2f  h=%.2f", t_shape.y,  t_shape.x, t_shape.width, t_shape.height), window->colors);

        GUI_Text(GUI_GridNextY(), "window_shape", window->colors);
        GUI_Text(GUI_GridNextY(), TextFormat("x=%.2f  y=%.2f w=%.2f  h=%.2f", w_shape.x, w_shape.y, w_shape.width, w_shape.height),  window->colors);
        GUI_Text(GUI_GridNextY(), "--- End focused window ---", window->colors);
    }

    Rectangle next = GUI_GridNextY();
    float icon_w = GUI_GetIconWidth();
    GUI_Face((Vector2){ next.x, next.y }, (float) icon_w / 2);
    GUI_Face((Vector2){ next.x + (float) icon_w / 2, next.y }, icon_w);
    GUI_Face((Vector2){ next.x + (float) icon_w / 2 + icon_w, next.y }, icon_w * 2);
    GUI_Icon(&icons->Dog, (Vector2){ next.x + (float) icon_w / 2 + icon_w * 3, next.y }, icon_w * 2, WHITE);
    GUI_GridNextY(); // Jump line
    GUI_GridNextY(); // Jump line

    static Texture2D image;
    if (image.id == 0) image = LoadTexture("art/abstractica.png");
    next = GUI_GridNextY();
    GUI_Image(image, (Rectangle){ next.x, next.y, next.width, 320 });
}
