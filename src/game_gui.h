#ifndef UNITY_BUILD
 #define UNITY_BUILD 0
 #include <stdio.h>
 #include <raylib.h>
 #include "rayext.h"
 #include "game_structs.h"
 #include "gui_setup.h"
 #include "gui_structs.h" 
 #include "gui.h" 
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

//
// SAMPLE WINDOW
//

// Define your draw window
void WIN_window(GUI_Window* window)
{
    // Prepare your data
    Game_WindowState *win_state = GAME_CTX.win_state;

    // Responsive height (if you require it)
    // GUI_WindowUpdateShapeForContent(window);
    
    // Get the setup as it allows access to theming setup->theme.red
    GUI_Setup *setup            = GUI_GetSetup();
    // Keep or modify colors
    GUI_ThemeColors colors      = window->colors;
    // Set your font
    EGUI_FontType font_type     = win_state->font_toggle ? EGUI_FontType_GUI: EGUI_FontType_Default;    
    // And define your UI
    Rectangle window_workspace  =
    GUI_BeginWindowContents(window, font_type);
        // A default layout with 3 columns
        GUI_BeginBlockCols(3, window_workspace, font_type);

        // 1st input (textbox)
        GUI_Text(GUI_NextHorizontal(), "Text", colors);
        GUI_Input(GUI_NextHorizontals(2), win_state->input_contents, EGUI_InputText, colors);

        // 2nd input for integer
        // TODO@dc: add min, max and parsing
        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "Int", colors);
        GUI_Input(GUI_NextHorizontals(2), win_state->input_int_contents, EGUI_InputInt, colors);

        // 3rd input for float
        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "Float", colors);
        GUI_Input(GUI_NextHorizontals(2), win_state->input_float_contents, EGUI_InputFloat, colors);

        // Wallpaper check (checkbox/switch)
        // With a theme.red color
        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "Wallpaper",  colors);
        GUI_Check(GUI_NextHorizontals(2), &win_state->checkbox_value, "ON", "OFF", setup->theme.red);

        // Font toggler
        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "Font",  colors);
        GUI_Check(GUI_NextHorizontals(2), &win_state->font_toggle, "GUI", "DEF", colors);
    GUI_EndWindowContents(window);
}

void WIN_layouts(GUI_Window* window)
{
    GUI_Setup* setup = GUI_GetSetup();
    EGUI_FontType font_type = EGUI_FontType_Default;
    float default_height = GUI_CalcDefaultHeightScaled(font_type);

    Rectangle window_workspace =
    GUI_BeginWindowContents(window, EGUI_FontType_Default);

        // First block
        GUI_BeginBlock(window_workspace.width, default_height);
        GUI_Text(GUI_NextVertical(), "Some sample layouts for imKairos", setup->theme.gray);

        // and more verticals of full width (can be written as Horizontals too, but requires
        // an explicit call to GUI_BeginBlock() to end each line)
        GUI_BeginControlScissor();
            float color_alpha = 0.9;
            DrawDebugRect(GUI_Relative(GUI_NextVertical()), ColorAlpha(BROWN, color_alpha));
            DrawDebugRect(GUI_Relative(GUI_NextVertical()), ColorAlpha(BEIGE, color_alpha));

            // 1/3rd and 2/3rds blocks
            GUI_BeginBlock(window_workspace.width / 3, default_height);
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(YELLOW, color_alpha));                    
            DrawDebugRect(GUI_Relative(GUI_NextHorizontals(2)), ColorAlpha(GREEN, color_alpha));

            // Second block
            // 3 horizontals of 1/3 of the available space
            GUI_BeginBlock(window_workspace.width / 3, default_height);
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(DARKGRAY, color_alpha));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(GRAY, color_alpha));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(LIGHTGRAY, color_alpha));
            
            // Prepare for a new block with 5 elements per row
            // You can send negative values to use AVAILABLE - YOUR_VALUE
            // Ex:
            // -default_height means take all space minus a default_height to insert a final row
            GUI_BeginBlock(window_workspace.width / 5, -default_height);
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(BLACK, 0.1));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(BLACK, 0.2));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(BLACK, 0.3));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(BLACK, 0.4));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(BLACK, 0.5));
            
            // Final row
            GUI_BeginBlock(window_workspace.width / 2, default_height);
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(RED, color_alpha));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(BLUE, color_alpha));
        EndScissorMode();
    GUI_EndWindowContents(window);
}



void WIN_character_debug(GUI_Window* window)
{
    Game_State *game_state      = GAME_CTX.state;
    Game_Character *ch          = &game_state->characters[game_state->current_character];
    GUI_ThemeColors colors      = window->colors;
    EGUI_FontType font_type     = EGUI_FontType_Default;

    Rectangle workspace = GUI_BeginWindowContents(window, font_type);
        GUI_BeginBlockCols(2, workspace, font_type);

        // Shape
        GUI_Text(GUI_NextHorizontal(), "shape.x", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.2f", ch->shape.x), colors);

        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "shape.y", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.2f", ch->shape.y), colors);

        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "shape.w", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.2f", ch->shape.width), colors);

        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "shape.h", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.2f", ch->shape.height), colors);

        // Color (RGB)
        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "color", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("r:%d g:%d b:%d a:%d",
            ch->color.r, ch->color.g, ch->color.b, ch->color.a), colors);

        // Movement
        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "movement.x", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.3f", ch->movement.x), colors);

        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "movement.y", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.3f", ch->movement.y), colors);

        // Animation time
        GUI_BeginDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "anim_time", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.3f", ch->anim_time), colors);

    GUI_EndWindowContents(window);
}


void WIN_winman(GUI_Window* window)
{
    GUI_State *state = GUI_GetState();
    GUI_Setup *setup = GUI_GetSetup();
    GUI_Icons *icons = GUI_GetIcons();
    EGUI_FontType font_type = EGUI_FontType_Default;
    float default_height = GUI_CalcDefaultHeightScaled(font_type);

    Rectangle window_workspace =
    GUI_BeginWindowContents(window, font_type);
        GUI_BeginBlock(window_workspace.width, default_height);

        static GUI_Window* win_window = NULL;
        if (GUI_Button(GUI_NextVertical(), "Open sample window", NULL, window->colors)) {
            if (win_window == NULL || win_window->id == 0) {
                win_window = GUI_OpenWindow(2, "Sample window", (Rectangle){ 20, 20, 300, 100 }, setup->theme.gray, &icons->Dog, false, WIN_window);
            }
        }
        static GUI_Window* win_layouts = NULL;
        if (GUI_Button(GUI_NextVertical(), "Open layouts window", NULL, window->colors)) {
            if (win_layouts == NULL || win_layouts->id == 0) {
                win_layouts = GUI_OpenWindow(3, "Layouts window", (Rectangle){ 20, 20, 300, 100 }, setup->theme.gray, &icons->Layouts, false, WIN_layouts);
            }
        }
        static GUI_Window* win_character_debug = NULL;
        if (GUI_Button(GUI_NextVertical(), "Open Character debug", NULL, window->colors)) {
            if (win_character_debug == NULL || win_character_debug->id == 0) {
                win_character_debug = GUI_OpenWindow(4, "Character debug", (Rectangle){ 20, 20, 300, 100 }, setup->theme.gray, &icons->Dog, false, WIN_character_debug);
            }
        }

        GUI_Text(GUI_NextVertical(), "--- Opened windows ---", window->colors);
        GUI_BeginDuplicateBlock();
        for (int i = 0; i < GUI_MAX_OPEN_WINS; ++i) {
            GUI_Window* win = &state->window_s[i];
            if (win->id == 0 || window->id == win->id) continue;
            if (GUI_Button(GUI_NextVertical(), TextFormat("%d - %s", win->id, win->title), NULL, window->colors)) {
                state->force_z_index = win->id;
            }
        }

        GUI_Text(GUI_NextVertical(), "--- Global values ---", window->colors);
        
        // NOTE:
        // GUI_IsPointerOverGui() is not safe to be called by a Window. It requires ALL windows to be processed beforehand. 
        // GUI_Text(GUI_NextVertical(), TextFormat("PointerOverGUI: %d", GUI_IsPointerOverGui()),  window->colors);

        GUI_Window *active = GUI_GetWindowByZindex(0);
        if (active != NULL) {
            Rectangle t_shape = GUI_WindowTitle(active->shape);
            Rectangle w_shape = active->shape;

            GUI_Text(GUI_NextVertical(), "--- Focused window ---", window->colors);
            GUI_Text(GUI_NextVertical(), TextFormat("ID=%d scroll=%.2f content_height=%.2f", active->id, active->scroll_offset, active->content_height),  window->colors);

            GUI_Text(GUI_NextVertical(), "title_shape", window->colors);
            GUI_Text(GUI_NextVertical(), TextFormat("x=%.2f  y=%.2f w=%.2f  h=%.2f", t_shape.y,  t_shape.x, t_shape.width, t_shape.height), window->colors);

            GUI_Text(GUI_NextVertical(), "window_shape", window->colors);
            GUI_Text(GUI_NextVertical(), TextFormat("x=%.2f  y=%.2f w=%.2f  h=%.2f", w_shape.x, w_shape.y, w_shape.width, w_shape.height),  window->colors);
            GUI_Text(GUI_NextVertical(), "--- End focused window ---", window->colors);
        }

        Rectangle next = GUI_NextVertical();
        float icon_w = GUI_GetIconWidth();
        GUI_Face((Vector2){ next.x, next.y }, (float) icon_w / 2);
        GUI_Face((Vector2){ next.x + (float) icon_w / 2, next.y }, icon_w);
        GUI_Face((Vector2){ next.x + (float) icon_w / 2 + icon_w, next.y }, icon_w * 2);
        GUI_Icon(&icons->Dog, (Vector2){ next.x + (float) icon_w / 2 + icon_w * 3, next.y }, icon_w * 2, WHITE);
        GUI_NextVertical(); // Jump line
        GUI_NextVertical(); // Jump line

        static Texture2D image;
        if (image.id == 0) image = LoadTexture("art/abstractica.png");
        next = GUI_NextVertical();
        GUI_Image(image, (Rectangle){ next.x, next.y, next.width, 320 });
    GUI_EndWindowContents(window);
}