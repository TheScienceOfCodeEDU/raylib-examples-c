#ifndef UNITY_BUILD
 #include <string.h>
 #include <stdio.h>
 #include "rayext.h"
 #include "str.h"
 #include "env.h"
 #include "gui_setup.h"
#endif

#define GUI_MAX_OPEN_WINS   16
#define GUI_MAX_TEXTBOXES   256

typedef enum {
    EGUI_Status_Default,
    EGUI_Status_Collide,
    EGUI_Status_Focused
} GUI_ElementStatus;

typedef enum {
    GUI_Focus_Available,
    GUI_Focus_CanOverride,
    GUI_Focus_Granted
} GUI_Focus;

// NOTE: define a function near the type instead of using it everywhere. 
//       Now, we know that the order matters for this Enum.
bool FocusOverridable(GUI_Focus focus)
{
    return focus <= GUI_Focus_CanOverride;
}

// - WINDOW STRUCTS -----------------------------------------------------------
//   PROGRESS : ██████████░  90%     STABILITY : █████████░  90%
//   STATUS   : Stable
//   NOTES    : Resize. Close and open.
// ----------------------------------------------------------------------------

#define MAX_WINDOW_TITLE 16

typedef struct GUI_Window {
    int             id;
    Rectangle       shape;
    GUI_ThemeColors colors;
    char            *title;
    Texture2D       *icon;
    void (*contents) (struct GUI_Window*, void*);
} GUI_Window;

GUI_Window GUI_MakeEmptyWindow(void)
{
    GUI_Window window = {
        .id       = 0,
        .shape    = (Rectangle){0, 0, 0, 0},
        .colors   = {{0}},
        .title    = NULL,
        .icon     = NULL,
        .contents = NULL
    };
    return window;
}

// - GUI STATE ----------------------------------------------------------------
//   PROGRESS : ███░░░░░░░░  30%     STABILITY : █████░░░░░  50%
//   STATUS   : Unstable
//   NOTES    : Save and restore.
// ----------------------------------------------------------------------------

typedef struct {
    RenderTexture2D buffer;
    float           scale;
    bool            window_focus_moving;
    int             control_focus_id;
    GUI_Focus       focus_state_current;

    EGUI_Pointer    current_pointer;
    Vector2         mouse_last;
    Vector2         mouse_current;

    GUI_Window      window_s[GUI_MAX_OPEN_WINS];
    int             force_z_index;
    int             z_index[GUI_MAX_OPEN_WINS];

    int             textbox_cursors[GUI_MAX_TEXTBOXES];
} GUI_State;

GUI_State GUI_MakeStateDefault(Vector2 screen_max)
{
    GUI_State state = {
        .buffer                 = LoadRenderTexture(screen_max.x, screen_max.y),
        .scale                  = 1.0f,
        .window_focus_moving    = false,
        .control_focus_id       = 0,
        .focus_state_current    = GUI_Focus_Available,

        .current_pointer        = EGUI_Pointer_Default,
        .mouse_last             = (Vector2){ 0.0f, 0.0f },
        .mouse_current          = (Vector2){ 0.0f, 0.0f }
    };

    for (int i = 0; i < GUI_MAX_OPEN_WINS; i++) {
        state.window_s[i] = GUI_MakeEmptyWindow();
    }

    state.force_z_index = 0;
    memset(state.z_index, 0, sizeof(state.z_index));
    memset(state.textbox_cursors, 0, sizeof(state.textbox_cursors));
    // SetTextureFilter(state.font.texture, TEXTURE_FILTER_POINT);
    return state;
}

// - GUI CONTEXT --------------------------------------------------------------
//   PROGRESS : ██████████░  90%     STABILITY : ███████░░░  90%
//   STATUS   : Stable
// ----------------------------------------------------------------------------

static struct {
    GUI_State* state;
    GUI_Setup* setup;

    // LAYOUT DATA
    // Vertical
    int    vertical_count;
    float  vertical_size;

    // Horizontal
    int    horizontal_count;
    float  horizontal_size;
} GUI_CTX = { 0 };

void GUI_SetContext(GUI_State* state, GUI_Setup* setup)
{
    GUI_CTX.setup = setup;
    GUI_CTX.state = state;
}

GUI_State* GUI_GetState()
{
    return GUI_CTX.state;
}

GUI_Setup* GUI_GetSetup()
{
    return GUI_CTX.setup;
}

GUI_FontSetup* GUI_GetFontSetup(EGUI_Content content)
{
    // TODO@dc: Add validations
    return &GUI_CTX.setup->font_setups[content];
}

Font GUI_GetFont(EGUI_Content content)
{
    GUI_Setup *setup = GUI_GetSetup();
    if (setup->font_setups[content].font_use_custom)
        return setup->font_setups[content].font_custom;
    else
        return GetFontDefault();
}

GUI_PointerSetup* GUI_GetPointerSetup()
{
    GUI_Setup* setup = GUI_GetSetup();
    GUI_State* state = GUI_GetState();

    EGUI_Pointer pointer = state->current_pointer;
    return &setup->pointer_setups[pointer];
}

float GUI_CalcDefaultHeightScaled(EGUI_Content content)
{
    GUI_Setup* setup = GUI_GetSetup();
    GUI_State* state = GUI_GetState();
    return setup->font_setups[content].default_height * state->scale;
}