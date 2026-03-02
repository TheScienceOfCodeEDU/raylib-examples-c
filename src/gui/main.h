#pragma once
#ifndef UNITY_BUILD
#define UNITY_BUILD 0
#include "../common.h"
#endif

#include "types.h"
#include "_layout.h"
#include "_setup.h"

// > FUNCTIONS
//   INDEX

// > CONSTRUCTORS

GUI_State           GUI_MakeStateDefault(Vector2 screen_max);

GUI_Overlay         GUI_MakeOverlay();
GUI_Temp            GUI_MakeTempDefault();

// > POINTER
bool                GUI_IsCursorOverGui();
bool                GUI_IsCurrentWindowTarget(int window_id);
bool                GUI_IsCursorOverOverlay();
GUI_FontSetup*      GUI_GetFontSetup(EGUI_Font font);
Font                GUI_GetFontAsset(EGUI_Font font);
GUI_CursorSetup*    GUI_GetCursorSetup();

// > WINDOW
GUI_Window*         GUI_GetWindow(int id);
GUI_Window*         GUI_GetWindowByZindex(int z);
void                GUI_ForceZindex(int win_id);
GUI_Window* GUI_OpenWindow(
    int id, const char *title, Rectangle shape,
    GUI_ThemeColors colors, Texture2D *icon, bool focused_face, void (*contents)(GUI_Window*));
void GUI_RemoveWindow(int id);

Rectangle   GUI_GetWindowTitle(Rectangle shape);
Rectangle   GUI_GetWindowPanel(Rectangle shape);
Rectangle GUI_GetWindowBottom(Rectangle shape);
void GUI_WindowUpdateShapeForContent(GUI_Window *window);
Rectangle GUI_MakeWorkspace();
Rectangle GUI_GetWindowWorkspace(GUI_Window *window);
static inline Rectangle GUI_Relative(Rectangle shape);
static inline EGUI_Font GUI_GetFont();

// > OVERLAY
//   API

static inline bool GUI_OverlayIsOpenBy(const char* text_id_owner);
static inline GUI_Overlay* GUI_OverlayGetDraw();
static inline bool GUI_OverlayGetJustInteracted();
static inline void GUI_OverlayClose();
static inline void GUI_OverlayOpenFor(const char* id);
static inline void GUI_OverlaySetDrawCall(
    bool just_interacted,
    void (*draw_function)(void));
static inline void GUI_OverlaySetShapeDrawed(Rectangle shape_drawed);
void GUI_CloseOverlayOnInteraction(bool force, Rectangle shape);

// > FRAME
//   PIPELINE

void GUI_BeginDraw(EGUI_Cursor cursor_style);

void GUI_EndDraw();




#include "_controls.h"
#include "_window.h"


// > FUNCTIONS
//   IMPLEMENTATION

#ifdef IMPLEMENT_ALL

// > CONSTRUCTORS
//   DEFAULTS

GUI_Window GUI_MakeEmptyWindow(void)
{
    GUI_Window window = {
        .id             = 0,
        .shape          = (Rectangle){ 0, 0, 0, 0 },
        .colors         = {{ 0 }},
        .title          = NULL,
        .icon           = NULL,
        .scroll_offset  = 0.0f,
        .content_height = 0.0f,
        .focused_face   = true,
        .contents       = NULL,
    };
    return window;
}

GUI_State GUI_MakeStateDefault(Vector2 screen_max)
{
    GUI_State state = {
        .buffer         = LoadRenderTexture((int)screen_max.x, (int)screen_max.y),
        .scale          = 2.0f,
        .force_z_index  = 0
    };

    // IMPORTANT: keep GUI buffer pixel-perfect
    SetTextureFilter(state.buffer.texture, TEXTURE_FILTER_POINT);

    for (int i = 0; i < GUI_MAX_OPEN_WINS; i++) {
        state.window_s[i] = GUI_MakeEmptyWindow();
    }

    memset(state.z_index, 0, sizeof(state.z_index));
    return state;
}



GUI_Overlay GUI_MakeOverlay()
{
    GUI_Overlay overlay = {
        .id_ptr             = NULL,
        .window_target_id   = 0,
        .layout             = GUI_MakeLayoutTemp(),
        .just_interacted    = false,
        .shape_drawed       = (Rectangle){0,0,0,0},
        .function           = NULL,
    };
    return overlay;
}

GUI_Temp GUI_MakeTempDefault()
{
    GUI_Temp temp = {
        .status             = EGUI_Status_Off,

        .cursor             = EGUI_Cursor_Default,
        .mouse_last         = (Vector2){ 0.0f, 0.0f },
        .mouse_current      = (Vector2){ 0.0f, 0.0f },
        .cursor_over_gui    = false,
        .cursor_trail       = {{0}},

        .overlay_draw       = GUI_MakeOverlay(),
        .window             = GUI_MakeWindowTemp(),
        .layout             = GUI_MakeLayoutTemp()

    };
    return temp;
}

// > CONTEXT
//   API

void GUI_SetContext(GUI_State* state, GUI_Setup* setup, GUI_Temp* temp)
{
    // Update statuses
    // Outgoing temp
    if (GUI_CTX.temp != NULL) {
        GUI_CTX.temp->status = EGUI_Status_Off;
    }
    // Incoming
    temp->status = EGUI_Status_Ready;

    GUI_CTX.setup = setup;
    GUI_CTX.state = state;
    GUI_CTX.temp  = temp;
}

// To be used out-side this module
GUI_State* GUI_GetState()
{
    return GUI_CTX.state;
}

// To be used out-side this module
GUI_Setup* GUI_GetSetup()
{
    return GUI_CTX.setup;
}


// > POINTER
//   STATE

// GUI_IsCursorOverGui() is meant to be called after EndDraw
// If you need it internally, it means that you're creating and internal component so you could use GUI_CTX.temp->cursor_over_gui
bool GUI_IsCursorOverGui()
{
    Assert(GUI_CTX.temp->status == EGUI_Status_Ready);
    return GUI_CTX.temp->cursor_over_gui;
}

// Returns true if window id is the interactable target (cursor is over and z-index is the lowest possible)
bool GUI_IsCurrentWindowTarget(int window_id)
{
    GUI_WindowTemp temp    = GUI_CTX.temp->window;
    return temp.window_target_id == 0 || temp.window_target_id == window_id;
}

bool GUI_IsCursorOverOverlay()
{
    bool is_cursor_over = CheckCollisionPointRec(GUI_CTX.temp->mouse_current, GUI_CTX.temp->overlay_draw.shape_drawed);
    return is_cursor_over;
}

GUI_FontSetup* GUI_GetFontSetup(EGUI_Font font)
{
    // TODO@dc: Add validations
    return &GUI_CTX.setup->fonts[font];
}

Font GUI_GetFontAsset(EGUI_Font font)
{
    GUI_Setup *setup = GUI_CTX.setup;
    if (setup->fonts[font].use_custom)
        return setup->fonts[font].custom;
    else
        return GetFontDefault();
}

GUI_CursorSetup* GUI_GetCursorSetup()
{
    EGUI_Cursor cursor = GUI_CTX.temp->cursor;
    return &GUI_CTX.setup->cursors[cursor];
}

// > WINDOW
//   MODEL & GEOMETRY

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
    if (z >= 0 && z < GUI_MAX_OPEN_WINS)
        return GUI_GetWindow(GUI_CTX.state->z_index[z]);
    else
        return NULL;
}

void GUI_ForceZindex(int win_id)
{
    GUI_CTX.state->force_z_index = win_id;
}

GUI_Window* GUI_OpenWindow(
    int id, const char *title, Rectangle shape,
    GUI_ThemeColors colors, Texture2D *icon, bool focused_face,
    void (*contents)(GUI_Window*))
{
    // avoid duplicates
    if (GUI_GetWindow(id)) return GUI_GetWindow(id);

    for (int i = 0; i < GUI_MAX_OPEN_WINS; ++i) {
        GUI_Window* window = &GUI_CTX.state->window_s[i];
        if (window->id == 0) {
            window->id              = id;
            window->shape           = shape;
            window->colors          = colors;
            window->title           = title;
            window->icon            = icon;
            window->focused_face    = focused_face;
            window->contents        = contents;
            return window;
        }
    }
    return NULL;
}

void GUI_RemoveWindow(int id)
{
    for (int i = 0; i < GUI_MAX_OPEN_WINS; ++i) {
        GUI_Window* window = &GUI_CTX.state->window_s[i];
        if (window->id == id) {
            window->id = 0;
            return;
        }
    }
}



// > WINDOW
//   MODEL & GEOMETRY
//   STABILITY : █████████░  90%
//   NOTES     : Nothing here
Rectangle GUI_GetWindowTitle(Rectangle shape)
{
    float border    = GUI_GetFontSetup(EGUI_Font_GUI)->border;
    float scale     = GUI_CTX.state->scale;

    Rectangle shape_title = {
        shape.x + border * scale,
        shape.y + border * scale,
        shape.width - (border * scale * 2),
        GUI_CalcDefaultHeightScaled(EGUI_Font_GUI)
    };
    return shape_title;
}

Rectangle GUI_GetWindowPanel(Rectangle shape)
{
    Rectangle shape_title = GUI_GetWindowTitle(shape);

    float border            = GUI_GetFontSetup(EGUI_Font_GUI)->border;
    float scale             = GUI_CTX.state->scale;
    float icon_sm_width     = GUI_GetIconSmallWidth();

    Vector2 close_position      = {
        shape_title.x + shape_title.width - icon_sm_width - border * scale,
        shape_title.y
    };
    Rectangle panel_position    = RectFromVector2(close_position, icon_sm_width + border * scale * 2, shape_title.height);
    panel_position              = AddRect(panel_position, - border * scale, 0.f, 0.f, 0.f);

    // Review last pixels right
    return panel_position;
}

Rectangle GUI_GetWindowBottom(Rectangle shape)
{
    float border            = GUI_GetFontSetup(EGUI_Font_GUI)->border;
    float scale             = GUI_CTX.state->scale;
    float bottom_height     = border * scale * 3;

    Rectangle shape_bottom = {
        shape.x,
        shape.y + shape.height - bottom_height,
        shape.width,
        bottom_height
    };
    return shape_bottom;
}

void GUI_WindowUpdateShapeForContent(GUI_Window *window)
{
    float border            = GUI_GetFontSetup(EGUI_Font_GUI)->border;
    float scale             = GUI_CTX.state->scale;

    Rectangle shape_title   = GUI_GetWindowTitle(window->shape);
    Rectangle shape_bottom  = GUI_GetWindowBottom(window->shape);
    float current_height    = window->content_height + (shape_title.y - window->shape.y) + border * scale * 2
                            + shape_title.height + shape_bottom.height;

    window->shape.height    = current_height;
}

Rectangle GUI_MakeWorkspace()
{
    Rectangle workspace = {
        0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()
    };
    return workspace;
}

Rectangle GUI_GetWindowWorkspace(GUI_Window *window)
{
    Rectangle shape         = window->shape;
    float content_height    = window->content_height;
    float border            = GUI_GetFontSetup(EGUI_Font_GUI)->border;
    float scale             = GUI_CTX.state->scale;

    Rectangle shape_title  = GUI_GetWindowTitle(shape);
    Rectangle shape_bottom = GUI_GetWindowBottom(shape);
    Rectangle shape_workspace = {
        .x          = shape_title.x,
        .y          = shape_title.y + shape_title.height + (shape_title.y - shape.y),
        .width      = shape.width - (shape_title.x - shape.x ) * 3,
        .height     = shape.height - shape_title.height - (shape_title.y - shape.y) - border * scale - shape_bottom.height
    };

    // Vertical scroll
    if (shape_workspace.height < content_height) {
        shape_workspace.width -= border * scale * 3;
    }
    return shape_workspace;
}


static inline EGUI_Font GUI_GetFont()
{
    EGUI_Font font = GUI_CTX.temp->layout.current_font;
    return font;
}


// > OVERLAY
//   API
//   STABILITY : █████████░  90%

static inline bool GUI_OverlayIsOpenBy(const char* text_id_owner)
{
    return GUI_CTX.temp->overlay_draw.id_ptr == text_id_owner;
}

static inline GUI_Overlay* GUI_OverlayGetDraw()
{
    return &GUI_CTX.temp->overlay_draw;
}

static inline bool GUI_OverlayGetJustInteracted()
{
    return GUI_CTX.temp->overlay_draw.just_interacted;
}

static inline void GUI_OverlayClose()
{
    GUI_CTX.temp->overlay_draw = GUI_MakeOverlay();
}

static inline void GUI_OverlayOpenFor(const char* id)
{
    Assert(id != NULL);
    GUI_CTX.temp->overlay_draw.id_ptr = id;
}

static inline void GUI_OverlaySetDrawCall(
    bool just_interacted,
    void (*draw_function)(void))
{
    GUI_CTX.temp->overlay_draw.layout           = GUI_CTX.temp->layout;
    GUI_CTX.temp->overlay_draw.window_target_id = GUI_CTX.temp->window.window_target_id;
    GUI_CTX.temp->overlay_draw.just_interacted  = just_interacted;
    GUI_CTX.temp->overlay_draw.function         = draw_function;
}

static inline void GUI_OverlaySetShapeDrawed(Rectangle shape_drawed)
{
    Assert(GUI_CTX.temp->overlay_draw.id_ptr != NULL);
    GUI_CTX.temp->overlay_draw.shape_drawed = shape_drawed;
}

void GUI_CloseOverlayOnInteraction(bool force, Rectangle shape)
{
    bool interacted     = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool interactable   = GUI_OverlayGetJustInteracted() == false;

    if (force || (interacted && interactable)) {
        GUI_ForceZindex(GUI_CTX.temp->overlay_draw.window_target_id);
        GUI_OverlayClose();
    } else {
        Rectangle relative_shape = GUI_RelativePositionOnly(shape);
        GUI_OverlaySetShapeDrawed(relative_shape);
        #if DEV_DEBUG_GUI == 1
        DrawDebugRect(relative_shape, RED);
        #endif
    }
}

// > FRAME
//   PIPELINE

void GUI_BeginDraw(EGUI_Cursor cursor_style)
{
    Assert(GUI_CTX.temp->status == EGUI_Status_Ready);
    GUI_CTX.temp->status = EGUI_Status_Drawing;

    GUI_CTX.temp->cursor            = cursor_style;
    GUI_CTX.temp->cursor_over_gui   = false;

    Rectangle mouse_limits = (Rectangle) {
        0,
        0,
        (float) GetScreenWidth(),
        (float) GetScreenHeight()
    };
    GUI_CTX.temp->mouse_current = LimitVector2Rect(GetMousePosition(), mouse_limits);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        GUI_CTX.temp->control_focus_ptr = NULL;
    }

    GUI_CTX.temp->layout                       = GUI_MakeLayoutTemp();
}

void GUI_DrawOverlay()
{
    GUI_Overlay *overlay    = &GUI_CTX.temp->overlay_draw;
    bool is_enabled             = overlay->function != NULL && overlay->id_ptr != NULL;

    if (is_enabled) {
        GUI_CTX.temp->layout                    = overlay->layout;
        GUI_CTX.temp->layout.force_overflow     = true;

        overlay->function();
        overlay->function = NULL;
    }
}

void GUI_EndDraw()
{
    GUI_CTX.temp->mouse_last = GUI_CTX.temp->mouse_current;
    GUI_DrawOverlay();

    Assert(GUI_CTX.temp->status == EGUI_Status_Drawing);
    GUI_CTX.temp->status = EGUI_Status_Ready;
}




#endif
