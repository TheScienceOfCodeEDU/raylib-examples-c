#ifndef UNITY_BUILD
 #include "gui_setup.h"
#endif

#define GUI_MAX_OPEN_WINS   16
#define GUI_MAX_TEXTBOXES   256
#define GUI_SCROLL_SPEED    4
#define GUI_NO_WIN          -1

// > ENUMS
//   STABILITY : █████████░  90%
//   NOTES     : Nothing here

typedef enum {
    EGUI_Status_Default,
    EGUI_Status_Collide,
    EGUI_Status_Focused
} GUI_ElementStatus;

typedef enum {
    EGUI_InputText,      // editable ASCII text
    EGUI_InputInt,       // integer numeric input
    EGUI_InputFloat      // floating numeric input
} EGUI_InputType;

/*
EXAMPLE: Enum order matters
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
*/


// > WINDOW STRUCTS
//   STABILITY : █████████░  90%
//   NOTES     : Resize. Close and open.

#define MAX_WINDOW_TITLE 16

typedef struct GUI_Window {
    int             id;
    Rectangle       shape;
    GUI_ThemeColors colors;
    char            *title;
    Texture2D       *icon;
    float           scroll_offset;
    float           content_height; // Automatically stored by GUI_EndWindowContents. Calculated during layout processing.
    bool            focused_face;
    void (*contents) (struct GUI_Window*, void*);
} GUI_Window;

GUI_Window GUI_MakeEmptyWindow(void)
{
    GUI_Window window = {
        .id             = 0,
        .shape          = (Rectangle){0, 0, 0, 0},
        .colors         = {{0}},
        .title          = NULL,
        .icon           = NULL,
        .scroll_offset  = 0.0f,
        .content_height = 0.0f,
        .focused_face   = true,
        .contents       = NULL,        
    };
    return window;
}

// > STATE
//   STABILITY : ███░░░░░░░  30%
//   NOTES     : Save and restore

typedef struct {
    RenderTexture2D buffer;
    float           scale;

    GUI_Window      window_s[GUI_MAX_OPEN_WINS];
    int             force_z_index;

    // z_index stores windows indexes or zero as empty.
    int             z_index[GUI_MAX_OPEN_WINS];
} GUI_State;

GUI_State GUI_MakeStateDefault(Vector2 screen_max)
{
    GUI_State state = {
        .buffer                 = LoadRenderTexture(screen_max.x, screen_max.y),
        .scale                  = 1.0f
    };

    for (int i = 0; i < GUI_MAX_OPEN_WINS; i++) {
        state.window_s[i] = GUI_MakeEmptyWindow();
    }

    state.force_z_index = 0;
    memset(state.z_index, 0, sizeof(state.z_index));
    // SetTextureFilter(state.font.texture, TEXTURE_FILTER_POINT);
    return state;
}

typedef struct {
    // Window runtime
    bool            window_focus_moving;
    void            *control_focus_ptr;
    int             window_coll_id;

    // Pointer runtime
    EGUI_Pointer    current_pointer;
    Vector2         mouse_last;
    Vector2         mouse_current;

    // Window that is being processed right now
    // This is NOT the active window focused by the player. Active win_idx is ==> GUI_State.z_index[0]
    int             current_window_idx;       // Current window being drawn
    float           current_scroll;
    EGUI_FontType   current_font_type;
    Rectangle       current_window_workspace; // Current window workspace

    // Layout temporary data
    Rectangle       current_layout_workspace; // Current available (Use only for layouts)
    int             vertical_count;
    float           vertical_size;
    int             horizontal_count;
    float           horizontal_size;
    float           layout_used_height;
} GUI_Temp;

GUI_Temp GUI_MakeTempDefault()
{
    GUI_Temp temp = {
        .window_focus_moving        = false,
        .control_focus_ptr          = NULL,
        .window_coll_id             = 0,

        .current_pointer            = EGUI_Pointer_Default,
        .mouse_last                 = (Vector2){ 0.0f, 0.0f },
        .mouse_current              = (Vector2){ 0.0f, 0.0f },

        .current_window_idx         = GUI_NO_WIN,
        .current_scroll             = 0,
        .current_font_type          = EGUI_FontType_Default,
        .current_window_workspace   = (Rectangle) { 0.f, 0.f, 0.f, 0.f},

        .current_layout_workspace   = (Rectangle) { 0.f, 0.f, 0.f, 0.f},
        .vertical_count             = 0,
        .vertical_size              = 0.0f,
        .horizontal_count           = 0,
        .horizontal_size            = 0.0f,
        .layout_used_height         = 0.0f
    };
    return temp;
}


// > STATE > CONTEXT
//   STABILITY : █████████░  90%
//   NOTES     : Nothing here

static struct {
    GUI_State*  state;
    GUI_Setup*  setup;
    GUI_Temp    temp;
} GUI_CTX = { 0 };

void GUI_SetContext(GUI_State* state, GUI_Setup* setup)
{
    GUI_CTX.setup = setup;
    GUI_CTX.state = state;
    GUI_CTX.temp  = GUI_MakeTempDefault();
}

GUI_State* GUI_GetState()
{
    return GUI_CTX.state;
}

GUI_Setup* GUI_GetSetup()
{
    return GUI_CTX.setup;
}

GUI_Icons* GUI_GetIcons()
{
    GUI_Setup *setup = GUI_GetSetup();
    return &setup->icon_setup.icons;
}

GUI_FontSetup* GUI_GetFontSetup(EGUI_FontType font_type)
{
    // TODO@dc: Add validations
    return &GUI_CTX.setup->font_setups[font_type];
}

Font GUI_GetFont(EGUI_FontType font_type)
{
    GUI_Setup *setup = GUI_GetSetup();
    if (setup->font_setups[font_type].font_use_custom)
        return setup->font_setups[font_type].font_custom;
    else
        return GetFontDefault();
}

GUI_PointerSetup* GUI_GetPointerSetup()
{
    GUI_Setup* setup = GUI_GetSetup();
    
    EGUI_Pointer pointer = GUI_CTX.temp.current_pointer;
    return &setup->pointer_setups[pointer];
}

GUI_Window* GUI_GetWindow(int id)
{
    if (id == GUI_NO_WIN) {
        return NULL;
    }

    for (int i = 0; i < GUI_MAX_OPEN_WINS; ++i) {
        GUI_Window* window = &GUI_CTX.state->window_s[i];
        if (window->id == id) {
            return window;
        }
    }
    return NULL;
}

GUI_Window* GUI_GetWindowByZindex(int z)
{
    return GUI_GetWindow(GUI_CTX.state->z_index[z]);
}

float GUI_CalcDefaultHeightScaled(EGUI_FontType font_type)
{
    GUI_Setup* setup = GUI_GetSetup();
    GUI_State* state = GUI_GetState();
    return setup->font_setups[font_type].default_height * state->scale;
}

// > WINDOW UTILS
//   STABILITY : █████████░  90%
//   NOTES     : Nothing here
Rectangle GUI_WindowTitle(Rectangle shape)
{
    float border    = GUI_GetFontSetup(EGUI_FontType_GUI)->border;
    float scale     = GUI_CTX.state->scale;

    Rectangle shapeTitle = {
        shape.x + border * scale,
        shape.y + border * scale,
        shape.width - (border * scale * 2),
        GUI_CalcDefaultHeightScaled(EGUI_FontType_GUI)
    };
    return shapeTitle;
}

void GUI_WindowUpdateShapeForContent(GUI_Window *window)
{
    float border            = GUI_GetFontSetup(EGUI_FontType_GUI)->border;
    float scale             = GUI_CTX.state->scale;

    Rectangle shape_title   = GUI_WindowTitle(window->shape);
    float current_height    = window->content_height + shape_title.height + (shape_title.y - window->shape.y) + border * scale * 2;
    window->shape.height    = current_height;
}

Rectangle GUI_WindowWorkspace(GUI_Window *window)
{
    Rectangle shape         = window->shape;
    float content_height    = window->content_height;
    float border            = GUI_GetFontSetup(EGUI_FontType_GUI)->border;
    float scale             = GUI_CTX.state->scale;

    Rectangle shape_title = GUI_WindowTitle(shape);
    Rectangle shape_workspace = {
        shape_title.x,
        shape_title.y + shape_title.height + (shape_title.y - shape.y),
        shape.width - (shape_title.x - shape.x ) * 2,
        shape.height - shape_title.height - (shape_title.y - shape.y) - border * scale * 2
    };

    // Vertical scroll
    if (shape_workspace.height < content_height) {
        shape_workspace.width -= border * scale * 3;
    }

    if (DEV_DEBUG_GUI) {
        DrawRectangleRec(shape_title, ColorAlpha(ORANGE, 0.5));
        DrawRectangleRec(shape_workspace, ColorAlpha(GREEN, 0.5));
    }
    return shape_workspace;
}

#define GUI_CONTROL_LAYOUT(shape) \
    if (GUI_CTX.temp.current_window_idx != GUI_NO_WIN) { \
        shape = RelativeToRect(shape, GUI_CTX.temp.current_layout_workspace); \
    };

#define GUI_CONTROL_FONT_TYPE_FROM_CONTEXT() \
    EGUI_FontType font_type = GUI_CTX.temp.current_font_type;



#define GUI_CONTROL_FOCUSED(value, shape)                                  \
    /* > GUI_CONTROL_FOCUSED                                             */\
    /*   is_pointer_over    : pointer currently within control bounds    */\
    /*   is_pointer_active  : user pressed mouse or enter key this frame */\
    /*   just_focused       : control gained focus on this frame         */\
    /*   is_focused         : control retains focus state                */\
    /* Conditions */ \
    bool is_pointer_over    = GUI_CheckCollisionPointerControl(shape, GUI_GetWindow(GUI_CTX.temp.current_window_idx)); \
    bool is_pointer_active  = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyEnterPressed();                          \
    \
    /* Gains focus */                                          \
    bool just_focused = is_pointer_over && is_pointer_active;  \
    if (just_focused) {                                        \
        GUI_CTX.temp.control_focus_ptr = value;                \
    }                                                          \
    \
    /* Focused control */                                      \
    bool is_focused = GUI_CTX.temp.control_focus_ptr == value; \
        

void GUI_BeginFontType(EGUI_FontType font_type)
{
    GUI_CTX.temp.current_font_type = font_type;
}

void GUI_EndFontType()
{
    GUI_CTX.temp.current_font_type = EGUI_FontType_Default;
}
