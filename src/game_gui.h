#include "gui/_window.h"
#ifndef UNITY_BUILD
 #define UNITY_BUILD 0
 #include "common.h"
 #include "gui.h"
 #include "game_structs.h"
#endif

//
// INDEX
//
void GUI_ProgramMenu();
void GUI_GameMenu();
void GUI_TopBar(Rectangle shape);
void WIN_Window(GUI_Window* window);
void WIN_Layouts(GUI_Window* window);
void WIN_CharacterDebug(GUI_Window* window);
void WIN_Settings(GUI_Window* window);
void WIN_Winman(GUI_Window* window);

//
// FUNCTIONS
//


void GUI_ProgramMenu()
{
    GUI_Setup *setup        = GUI_GetSetup();
    GUI_Icons *icons        = GUI_GetIcons();
    GUI_Theme *theme        = &setup->theme;

    //  GUI_Button(GUI_NextInPlace(-1, 0),      "Project [X]",  NULL,           theme->red);
    GUI_Button(GUI_NextInPlace(-1,1),       "New",          &icons->New,    theme->gray);
    GUI_Button(GUI_NextInPlace(-1,2),       "Open",         &icons->Open,   theme->gray);
    if (GUI_Button(GUI_NextInPlace(-1,3),   "Quit",         &icons->New,    theme->gray))
        exit(0);

    GUI_CloseOverlayOnInteraction(GUI_NextInPlaceBetween(-1, 1, -1, 3));
}

void GUI_GameMenu()
{
    GUI_Setup *setup        = GUI_GetSetup();
    GUI_Icons *icons        = GUI_GetIcons();
    GUI_Theme *theme        = &setup->theme;
    PLAYER_Actions *actions = &GAME_CTX.temp->player_actions;

    //GUI_Button(GUI_NextInPlace(-1, 0), "Game [X]", &icons->Dog, theme->gray);
    actions->reset_characters    = GUI_Button(GUI_NextInPlace(-1,1), "Reset",    &icons->New,    theme->gray);
    actions->add_character       = GUI_Button(GUI_NextInPlace(-1,2), "Add",      &icons->Open,   theme->gray);
    actions->toggle_character    = GUI_Button(GUI_NextInPlace(-1,3), "Change",   &icons->Error,  theme->gray);

    GUI_CloseOverlayOnInteraction(GUI_NextInPlaceBetween(-1, 1, -1, 3));
}

void GUI_TopBar(Rectangle shape)
{
    GUI_Setup *setup    = GUI_GetSetup();
    GUI_Icons *icons    = GUI_GetIcons();
    GUI_Theme *theme    = &setup->theme;
    const int BUTTONS   = 4;

    GUI_LayoutReset(GUI_MakeWorkspace());
    GUI_LayoutBlockCols(BUTTONS, shape, EGUI_Font_GUI);
    GUI_ButtonMenu(GUI_NextHorizontal(), "Project",     NULL,           theme->red,     GUI_ProgramMenu);
    GUI_ButtonMenu(GUI_NextHorizontal(), "Game",        &icons->Dog,    theme->gray,    GUI_GameMenu);
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
    // And define your UI
    Rectangle window_workspace  =
    GUI_BeginWindowContents(window, font);
        // A default layout with 3 columns
        GUI_LayoutBlockCols(3, window_workspace, font);

        GUI_NextHorizontal();
        GUI_ButtonMenu(GUI_NextHorizontal(), "Game 2",    &icons->Dog,    theme.gray, GUI_GameMenu);

        GUI_LayoutDuplicateBlock();
        // 1st input (textbox)
        GUI_Text(GUI_NextHorizontal(), "Text", colors);
        GUI_Input(GUI_NextHorizontals(2), win_state->input_contents, EGUI_Input_Text, colors);

        // 2nd input for integer
        // TODO@dc: add min, max and parsing
        GUI_LayoutDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "Int", colors);
        GUI_Input(GUI_NextHorizontals(2), win_state->input_int_contents, EGUI_Input_Int, colors);

        // 3rd input for float
        GUI_LayoutDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "Float", colors);
        GUI_Input(GUI_NextHorizontals(2), win_state->input_float_contents, EGUI_Input_Float, colors);

        // Wallpaper check (checkbox/switch)
        // With a theme.red color
        GUI_LayoutDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "Wallpaper",  colors);
        GUI_Check(GUI_NextHorizontals(2), &win_state->checkbox_value, "ON", "OFF", setup->theme.red);

        // Font toggler
        GUI_LayoutDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "Font",  colors);
        GUI_Check(GUI_NextHorizontals(2), &win_state->font_toggle, "GUI", "DEF", colors);


    GUI_EndWindowContents(window);
}

void WIN_Layouts(GUI_Window* window)
{
    GUI_Setup* setup = GUI_GetSetup();
    EGUI_Font font = EGUI_Font_Default;
    float default_height = GUI_CalcDefaultHeightScaled(font);

    Rectangle window_workspace =
    GUI_BeginWindowContents(window, EGUI_Font_Default);

        // First block
        GUI_LayoutBlock(window_workspace.width, default_height);
        GUI_Text(GUI_NextVertical(), "Some sample layouts for imKairos", setup->theme.gray);

        // and more verticals of full width (can be written as Horizontals too, but requires
        // an explicit call to GUI_BeginBlock() to end each line)
        GUI_BeginControlScissor();
            float color_alpha = 0.9f;
            DrawDebugRect(GUI_Relative(GUI_NextVertical()), ColorAlpha(BROWN, color_alpha));
            DrawDebugRect(GUI_Relative(GUI_NextVertical()), ColorAlpha(BEIGE, color_alpha));

            // 1/3rd and 2/3rds blocks
            GUI_LayoutBlock(window_workspace.width / 3, default_height);
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(YELLOW, color_alpha));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontals(2)), ColorAlpha(GREEN, color_alpha));

            // Second block
            // 3 horizontals of 1/3 of the available space
            GUI_LayoutBlock(window_workspace.width / 3, default_height);
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(DARKGRAY, color_alpha));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(GRAY, color_alpha));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(LIGHTGRAY, color_alpha));

            // Prepare for a new block with 5 elements per row
            // You can send negative values to use AVAILABLE - YOUR_VALUE
            // Ex:
            // -default_height means take all space minus a default_height to insert a final row
            GUI_LayoutBlock(window_workspace.width / 5, -default_height);
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(BLACK, 0.1f));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(BLACK, 0.2f));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(BLACK, 0.3f));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(BLACK, 0.4f));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(BLACK, 0.5f));

            // Final row
            GUI_LayoutBlock(window_workspace.width / 2, default_height);
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(RED, color_alpha));
            DrawDebugRect(GUI_Relative(GUI_NextHorizontal()), ColorAlpha(BLUE, color_alpha));
        EndScissorMode();
    GUI_EndWindowContents(window);
}


void WIN_CharacterDebug(GUI_Window* window)
{
    GAME_State *game_state      = GAME_CTX.state;
    GAME_Character *ch          = &game_state->characters[game_state->current_character];
    GUI_ThemeColors colors      = window->colors;
    EGUI_Font font     = EGUI_Font_Default;

    Rectangle workspace = GUI_BeginWindowContents(window, font);
        GUI_LayoutBlockCols(2, workspace, font);

        // Shape
        GUI_Text(GUI_NextHorizontal(), "shape.x", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.2f", ch->shape.x), colors);

        GUI_LayoutDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "shape.y", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.2f", ch->shape.y), colors);

        GUI_LayoutDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "shape.w", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.2f", ch->shape.width), colors);

        GUI_LayoutDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "shape.h", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.2f", ch->shape.height), colors);

        // Color (RGB)
        GUI_LayoutDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "color", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("r:%d g:%d b:%d a:%d",
            ch->color.r, ch->color.g, ch->color.b, ch->color.a), colors);

        // Movement
        GUI_LayoutDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "movement.x", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.3f", ch->movement.x), colors);

        GUI_LayoutDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "movement.y", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.3f", ch->movement.y), colors);

        // Animation time
        GUI_LayoutDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "anim_time", colors);
        GUI_Text(GUI_NextHorizontal(), TextFormat("%.3f", ch->anim_time), colors);

    GUI_EndWindowContents(window);
}

// Define your draw window
void WIN_Settings(GUI_Window* window)
{
    GUI_State *state            = GUI_GetState();
    GUI_ThemeColors colors      = window->colors;
    EGUI_Font font     = EGUI_Font_Default;

    Rectangle window_workspace  =
    GUI_BeginWindowContents(window, font);
        GUI_LayoutBlockCols(3, window_workspace, font);
        GUI_Text(GUI_NextHorizontal(), "Scale", colors);
        GUI_Float(GUI_NextHorizontals(2), &state->scale, colors, 0.5f, 6.0f);

        GUI_LayoutDuplicateBlock();
        GUI_Text(GUI_NextHorizontal(), "Scale 2", colors);
        GUI_Float(GUI_NextHorizontals(2), &state->scale, colors, 0.5f, 6.0f);
    GUI_EndWindowContents(window);
}

void WIN_Winman(GUI_Window* window)
{
    GUI_State *state = GUI_GetState();
    GUI_Setup *setup = GUI_GetSetup();
    GUI_Icons *icons = GUI_GetIcons();
    EGUI_Font font = EGUI_Font_Default;
    float default_height = GUI_CalcDefaultHeightScaled(font);

    Rectangle window_workspace =
    GUI_BeginWindowContents(window, font);
        GUI_LayoutBlock(window_workspace.width, default_height);

        static GUI_Window* win_window = NULL;
        if (GUI_Button(GUI_NextVertical(), "Sample window", NULL, window->colors)) {
            int win_id = 2;
            if (win_window == NULL || win_window->id == 0) {
                win_window = GUI_OpenWindow(win_id, "Sample window", setup->theme.gray, &icons->Dog, false, WIN_Window);
            }
            GUI_ForceZindex(win_id);
        }
        static GUI_Window* win_layouts = NULL;
        if (GUI_Button(GUI_NextVertical(), "Layouts window", NULL, window->colors)) {
            int win_id = 3;
            if (win_layouts == NULL || win_layouts->id == 0) {
                win_layouts = GUI_OpenWindow(win_id, "Layouts window", setup->theme.gray, &icons->Layouts, false, WIN_Layouts);
            }
            GUI_ForceZindex(win_id);
        }
        static GUI_Window* win_character_debug = NULL;
        if (GUI_Button(GUI_NextVertical(), "Character debug", NULL, window->colors)) {
            int win_id = 4;
            if (win_character_debug == NULL || win_character_debug->id == 0) {
                win_character_debug = GUI_OpenWindow(win_id, "Character debug", setup->theme.gray, &icons->Dog, false, WIN_CharacterDebug);
            }
            GUI_ForceZindex(win_id);
        }
        static GUI_Window* win_settings = NULL;
        if (GUI_Button(GUI_NextVertical(), "Settings", &icons->Setup, window->colors)) {
            int win_id = 5;
            if (win_settings == NULL || win_settings->id == 0) {
                win_settings = GUI_OpenWindow(win_id, "Settings", setup->theme.gray, &icons->Face, true, WIN_Settings);
            }
            GUI_ForceZindex(win_id);
        }


        GUI_Text(GUI_NextVertical(), "--- Opened windows ---", window->colors);
        GUI_LayoutDuplicateBlock();
        for (int i = 0; i < GUI_MAX_OPEN_WINS; ++i) {
            GUI_Window* win = &state->window_s[i];
            if (win->id == 0 || window->id == win->id) continue;
            if (GUI_Button(GUI_NextVertical(), TextFormat("%d - %s", win->id, win->title), NULL, window->colors)) {
                GUI_ForceZindex(win->id);
            }
        }

        GUI_Text(GUI_NextVertical(), "--- Global values ---", window->colors);

        // NOTE:
        // GUI_IsPointerOverGui() is not safe to be called by a Window. It requires ALL windows to be processed beforehand.
        // GUI_Text(GUI_NextVertical(), TextFormat("PointerOverGUI: %d", GUI_IsPointerOverGui()),  window->colors);

        GUI_Window *active = GUI_GetWindowByZindex(0);
        if (active != NULL) {
            Rectangle t_shape = GUI_GetWindowTitle(active->shape);
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

