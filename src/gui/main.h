#pragma once
#ifndef UNITY_BUILD
 #define UNITY_BUILD 0
 #define IMPLEMENT_ALL 1
 #include "../common.h"
#endif


#include "types.h"
#include "_layout.h"
#include "_setup.h"

// > FUNCTIONS
//   INDEX

// > CONSTRUCTORS
//   DEFAULTS

GUI_Window GUI_MakeEmptyWindow(void);
GUI_State GUI_MakeStateDefault(Vector2 screen_max);

GUI_OverlayDraw GUI_MakeOverlayDraw();
GUI_Temp GUI_MakeTempDefault();



// > POINTER
//   STATE

bool GUI_IsPointerOverGui();
bool GUI_IsCurrentWindowTarget(int window_id);
bool GUI_IsPointerOverOverlay();
GUI_FontSetup* GUI_GetFontSetup(EGUI_FontType font_type);
Font GUI_GetFont(EGUI_FontType font_type);
GUI_PointerSetup* GUI_GetPointerSetup();

// > WINDOW
//   MODEL & GEOMETRY

GUI_Window* GUI_GetWindow(int id);
GUI_Window* GUI_GetWindowByZindex(int z);
void GUI_ForceZindex(int win_id);
GUI_Window* GUI_OpenWindow(
    int id, const char *title, Rectangle shape,
    GUI_ThemeColors colors, Texture2D *icon, bool focused_face,
    void (*contents)(GUI_Window*));
void GUI_RemoveWindow(int id);

Rectangle GUI_WindowTitle(Rectangle shape);
Rectangle GUI_WindowPanel(Rectangle shape);
Rectangle GUI_WindowBottom(Rectangle shape);
void GUI_WindowUpdateShapeForContent(GUI_Window *window);
Rectangle GUI_Workspace();
Rectangle GUI_WindowWorkspace(GUI_Window *window);
static inline Rectangle GUI_Relative(Rectangle shape);
static inline EGUI_FontType GUI_GetFontType();

// > OVERLAY
//   API

static inline bool GUI_OverlayIsOpenBy(const char* text_id_owner);
static inline GUI_OverlayDraw* GUI_OverlayGetDraw();
static inline bool GUI_OverlayGetJustInteracted();
static inline void GUI_OverlayClose();
static inline void GUI_OverlayOpenFor(const char* id);
static inline void GUI_OverlaySetDrawCall(
    bool just_interacted,
    void (*draw_function)(void));
static inline void GUI_OverlaySetShapeDrawed(Rectangle shape_drawed);



// > FRAME
//   PIPELINE

void GUI_BeginDraw(EGUI_Pointer pointer_style);
void GUI_DrawOverlay();
void GUI_EndDraw();


// > WINDOW
//   UI COMPOSITION

void GUI_WindowButtonPanel(GUI_Window* window, EGUI_FontType font_type);
void GUI_WindowEndingPanel(GUI_Window* window, EGUI_FontType font_type);
void GUI_DrawWindow(GUI_Window* window,  EGUI_ControlStatus status, EGUI_FontType font_type);
void GUI_UpdateAndDrawWindow(GUI_Window *window, Rectangle limits);
void GUI_CleanAndPrepareZIndex();
void GUI_UpdateAndDrawWindows(Rectangle limits);
Rectangle GUI_BeginWindowContents(GUI_Window* window, EGUI_FontType font_type);
void GUI_EndWindowContents(GUI_Window* window);

#include "_controls.h"


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

    for (int i = 0; i < GUI_MAX_OPEN_WINS; i++) {
        state.window_s[i] = GUI_MakeEmptyWindow();
    }

    memset(state.z_index, 0, sizeof(state.z_index));
    return state;
}



GUI_OverlayDraw GUI_MakeOverlayDraw()
{
    GUI_OverlayDraw overlay = {
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
        .status                     = EGUI_Status_Off,
        .current_action             = EGUI_WinActionNone,
        .control_focus_ptr          = NULL,
        .window_target_id           = 0,

        .current_pointer            = EGUI_Pointer_Default,
        .mouse_last                 = (Vector2){ 0.0f, 0.0f },
        .mouse_current              = (Vector2){ 0.0f, 0.0f },
        .pointer_over_gui           = false,
        .pointer_trail              = {{0}},

        .overlay_draw               = GUI_MakeOverlayDraw(),
        .layout                     = GUI_MakeLayoutTemp()
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

void GUI_SetFontType(EGUI_FontType font_type)
{
    GUI_CTX.temp->layout.current_font_type  = font_type;
}

GUI_Icons* GUI_GetIcons()
{
    return &GUI_CTX.setup->icon_setup.icons;
}

float GUI_GetIconWidth()
{
    return GUI_CTX.setup->icon_setup.icon_size * GUI_CTX.state->scale;
}

float GUI_GetIconWidthForShape(Rectangle shape, float border)
{
    return shape.height - border * 2 * GUI_CTX.state->scale;
}

float GUI_GetIconSmallWidth()
{
    return GUI_CTX.setup->icon_setup.icon_size_sm * GUI_CTX.state->scale;
}

// > POINTER
//   STATE

// GUI_IsPointerOverGui() is meant to be called after EndDraw
// If you need it internally, it means that you're creating and internal component so you could use GUI_CTX.temp->pointer_over_gui
bool GUI_IsPointerOverGui()
{
    Assert(GUI_CTX.temp->status == EGUI_Status_Ready);
    return GUI_CTX.temp->pointer_over_gui;
}

// Returns true if window id is the interactable target (pointer is over and z-index is the lowest possible)
bool GUI_IsCurrentWindowTarget(int window_id)
{
    return GUI_CTX.temp->window_target_id == 0 || GUI_CTX.temp->window_target_id == window_id;
}

bool GUI_IsPointerOverOverlay()
{
    bool is_pointer_over = CheckCollisionPointRec(GUI_CTX.temp->mouse_current, GUI_CTX.temp->overlay_draw.shape_drawed);
    return is_pointer_over;
}

GUI_FontSetup* GUI_GetFontSetup(EGUI_FontType font_type)
{
    // TODO@dc: Add validations
    return &GUI_CTX.setup->font_setups[font_type];
}

Font GUI_GetFont(EGUI_FontType font_type)
{
    GUI_Setup *setup = GUI_CTX.setup;
    if (setup->font_setups[font_type].font_use_custom)
        return setup->font_setups[font_type].font_custom;
    else
        return GetFontDefault();
}

GUI_PointerSetup* GUI_GetPointerSetup()
{
    EGUI_Pointer pointer = GUI_CTX.temp->current_pointer;
    return &GUI_CTX.setup->pointer_setups[pointer];
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
Rectangle GUI_WindowTitle(Rectangle shape)
{
    float border    = GUI_GetFontSetup(EGUI_FontType_GUI)->border;
    float scale     = GUI_CTX.state->scale;

    Rectangle shape_title = {
        shape.x + border * scale,
        shape.y + border * scale,
        shape.width - (border * scale * 2),
        GUI_CalcDefaultHeightScaled(EGUI_FontType_GUI)
    };
    return shape_title;
}

Rectangle GUI_WindowPanel(Rectangle shape)
{
    Rectangle shape_title = GUI_WindowTitle(shape);

    float border            = GUI_GetFontSetup(EGUI_FontType_GUI)->border;
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

Rectangle GUI_WindowBottom(Rectangle shape)
{
    float border            = GUI_GetFontSetup(EGUI_FontType_GUI)->border;
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
    float border            = GUI_GetFontSetup(EGUI_FontType_GUI)->border;
    float scale             = GUI_CTX.state->scale;

    Rectangle shape_title   = GUI_WindowTitle(window->shape);
    Rectangle shape_bottom  = GUI_WindowBottom(window->shape);
    float current_height    = window->content_height + (shape_title.y - window->shape.y) + border * scale * 2
                            + shape_title.height + shape_bottom.height;

    window->shape.height    = current_height;
}

Rectangle GUI_Workspace()
{
    Rectangle workspace = {
        0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()
    };
    return workspace;
}
Rectangle GUI_WindowWorkspace(GUI_Window *window)
{
    Rectangle shape         = window->shape;
    float content_height    = window->content_height;
    float border            = GUI_GetFontSetup(EGUI_FontType_GUI)->border;
    float scale             = GUI_CTX.state->scale;

    Rectangle shape_title  = GUI_WindowTitle(shape);
    Rectangle shape_bottom = GUI_WindowBottom(shape);
    Rectangle shape_workspace = {
        shape_title.x,
        shape_title.y + shape_title.height + (shape_title.y - shape.y),
        shape.width - (shape_title.x - shape.x ) * 3,
        shape.height - shape_title.height - (shape_title.y - shape.y) - border * scale - shape_bottom.height
    };

    // Vertical scroll
    if (shape_workspace.height < content_height) {
        shape_workspace.width -= border * scale * 3;
    }
    return shape_workspace;
}


static inline Rectangle GUI_Relative(Rectangle shape)
{
    bool is_active_layout = GUI_CTX.temp->layout.current_workspace.width  > 0 &&
                            GUI_CTX.temp->layout.current_workspace.height > 0;
    if (is_active_layout) {
        shape = RelativeToRect(shape, GUI_CTX.temp->layout.current_workspace);
    }
    return shape;
}

static inline EGUI_FontType GUI_GetFontType()
{
    EGUI_FontType font_type = GUI_CTX.temp->layout.current_font_type;
    return font_type;
}


// > OVERLAY
//   API
//   STABILITY : █████████░  90%

static inline bool GUI_OverlayIsOpenBy(const char* text_id_owner)
{
    return GUI_CTX.temp->overlay_draw.id_ptr == text_id_owner;
}

static inline GUI_OverlayDraw* GUI_OverlayGetDraw()
{
    return &GUI_CTX.temp->overlay_draw;
}

static inline bool GUI_OverlayGetJustInteracted()
{
    return GUI_CTX.temp->overlay_draw.just_interacted;
}

static inline void GUI_OverlayClose()
{
    GUI_CTX.temp->overlay_draw = GUI_MakeOverlayDraw();
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
    GUI_CTX.temp->overlay_draw.window_target_id = GUI_CTX.temp->window_target_id;
    GUI_CTX.temp->overlay_draw.just_interacted  = just_interacted;
    GUI_CTX.temp->overlay_draw.function         = draw_function;
}

static inline void GUI_OverlaySetShapeDrawed(Rectangle shape_drawed)
{
    Assert(GUI_CTX.temp->overlay_draw.id_ptr != NULL);
    GUI_CTX.temp->overlay_draw.shape_drawed = shape_drawed;
}



#include "_controls.h"

// > FRAME
//   PIPELINE

void GUI_BeginDraw(EGUI_Pointer pointer_style)
{
    Assert(GUI_CTX.temp->status == EGUI_Status_Ready);
    GUI_CTX.temp->status = EGUI_Status_Drawing;

    GUI_CTX.temp->current_pointer        = pointer_style;
    GUI_CTX.temp->pointer_over_gui       = false;

    Rectangle mouse_limits = (Rectangle) {
        0,
        0,
        (float) GetScreenWidth(),
        (float) GetScreenHeight()
    };
    GUI_CTX.temp->mouse_current = LimitVector2Rect(GetMousePosition(), mouse_limits);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        GUI_CTX.temp->control_focus_ptr      = NULL;
    }

    GUI_CTX.temp->layout                       = GUI_MakeLayoutTemp();
}

void GUI_DrawOverlay()
{
    GUI_OverlayDraw *overlay    = &GUI_CTX.temp->overlay_draw;
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



// > WINDOW
//   UI COMPOSITION
//   STABILITY : █████████░  90%
//   NOTES     : Nothing here

void GUI_WindowButtonPanel(GUI_Window* window, EGUI_FontType font_type)
{
    GUI_Icons *icons            = GUI_GetIcons();
    GUI_FontSetup *font_setup   = GUI_GetFontSetup(font_type);
    GUI_ThemeColors colors      = window->colors;

    float border                = font_setup->border;
    float scale                 = GUI_CTX.state->scale;
    float icon_sm_width         = GUI_GetIconSmallWidth();

    Rectangle shape_panel       = GUI_WindowPanel(window->shape);
    Vector2 position_button     = (Vector2) { shape_panel.x + border * scale, shape_panel.y };

    DrawRectangleRec(shape_panel, colors.bg_color_0);
    if (GUI_IconButton(&icons->CloseSmall, position_button, icon_sm_width, WHITE)) {
        GUI_RemoveWindow(window->id);
    }
    GUI_Icon(&icons->MinimizeSmall, AddVector2(position_button, 0, icon_sm_width + border * scale), icon_sm_width, WHITE);
}

void GUI_WindowEndingPanel(GUI_Window* window, EGUI_FontType font_type)
{
    GUI_FontSetup *font_setup   = GUI_GetFontSetup(font_type);
    GUI_ThemeColors colors      = window->colors;

    float border        = font_setup->border;
    float scale         = GUI_CTX.state->scale;

    Rectangle shape_bottom   = GUI_WindowBottom(window->shape);
    DrawRectangleRec((Rectangle) {
        shape_bottom.x + border * scale,
        shape_bottom.y,
        shape_bottom.width - border * scale * 2,
        border * scale
    }, colors.bg_color_2);
    DrawRectangleRec((Rectangle) {
        shape_bottom.x + border * scale,
        shape_bottom.y + border * scale,
        shape_bottom.width - border * scale * 2,
        border * scale
    }, colors.bg_color_0);
}

void GUI_DrawWindow(GUI_Window* window,  EGUI_ControlStatus status, EGUI_FontType font_type)
{
    GUI_State       *state          = GUI_CTX.state;
    GUI_FontSetup   *font_setup     = GUI_GetFontSetup(font_type);

    Rectangle        shape          = window->shape;
    Rectangle        shape_title    = GUI_WindowTitle(window->shape);
    Rectangle        shape_panel    = GUI_WindowPanel(window->shape);
    Rectangle        shape_bottom   = GUI_WindowBottom(window->shape);
    GUI_ThemeColors  colors         = window->colors;
    GUI_Theme        *theme         = &GUI_CTX.setup->theme;

    float border        = font_setup->border;
    float scale         = state->scale;
    float color_change  = theme->color_change;
    float bg_alpha      = theme->bg_alpha;

    // Background
    DrawRectangleRec((Rectangle){shape.x + border * scale, shape.y + border * scale, shape.width - border * scale, shape.height - 2 * border * scale}, ColorAlpha(colors.bg_color_1, bg_alpha));
    GUI_DrawBorders(shape, colors.bg_color_0, colors.bg_color_2, border * scale, true);

    if (status == EGUI_ControlStatus_Default) {
        DrawRectangleRec(shape_title, ColorAlpha(colors.bg_color_2, bg_alpha));
        GUI_DrawBorders(shape_title, colors.bg_color_2, colors.bg_color_0, border * scale, false);
    } if (status == EGUI_ControlStatus_Focused) {
        DrawRectangleRec(shape_title, ColorAlpha(ColorBrightness(colors.bg_color_3, -color_change), bg_alpha));
        GUI_DrawBorders(shape_title, colors.bg_color_2, colors.bg_color_0, border * scale, false);
        GUI_DrawBorders(shape, colors.bg_color_0, ColorBrightness(colors.bg_color_3,-color_change), border * scale, true);
    } if (status == EGUI_ControlStatus_Collide) {
        DrawRectangleRec(shape_title, ColorAlpha(ColorBrightness(colors.bg_color_3, color_change), bg_alpha));
        GUI_DrawBorders(shape_title, colors.bg_color_2, colors.bg_color_0, border * scale, false);
        GUI_DrawBorders(shape, colors.bg_color_0, ColorBrightness(colors.bg_color_3, color_change), border * scale, true);
    }

    bool reserve_icon_space = window->icon != NULL || (status == EGUI_ControlStatus_Focused && window->focused_face);
    float icon_w = reserve_icon_space ? GUI_GetIconWidth() : 0;

    GUI_BeginInnerControlScissor(shape_title, border, scale);
        GUI_DrawAdjustedTextEx(window->title,
            (Vector2) { shape_title.x + icon_w + (border) * scale, shape_title.y + (border) * scale },
            colors.tx_color_0, scale, EGUI_FontType_GUI);
    EndScissorMode();

    Vector2 icon_position = { shape_title.x + border * scale, shape_title.y + border * scale };
    if (window->icon != NULL && icon_w > 0) {
        GUI_Icon(window->icon, icon_position, icon_w, WHITE);
    }
    if (status == EGUI_ControlStatus_Focused && icon_w > 0 && window->focused_face) {
        GUI_Face((Vector2) { shape_title.x + border * scale, shape_title.y + border * scale }, icon_w);
    }

    // Vertical scroll
    // Scrollbar
    Rectangle workspace = GUI_WindowWorkspace(window);
    if (workspace.height < window->content_height) {
        float ratio = workspace.height / window->content_height;
        float bar_h = ratio * workspace.height;
        float bar_y = workspace.y + (window->scroll_offset / window->content_height) * workspace.height;

        DrawRectangleRec((Rectangle){
            workspace.x + workspace.width,
            workspace.y,
            border * scale * 3,
            workspace.height
        }, colors.bg_color_2);
        DrawRectangleRec((Rectangle){
            workspace.x + workspace.width + border * scale,
            bar_y,
            border * scale,
            bar_h
        }, colors.tx_color_0);
    }

    if (DEV_DEBUG_GUI) {
        DrawDebugRect(shape_bottom, ColorAlpha(YELLOW, 0.85));
        DrawDebugRect(shape_title, ColorAlpha(WHITE, 0.75));
        DrawDebugRect(shape_panel, ColorAlpha(RED, 0.85));
        DrawDebugRect(GUI_WindowWorkspace(window), ColorAlpha(GREEN, 0.25));
    }
}

void GUI_UpdateAndDrawWindow(GUI_Window *window, Rectangle limits)
{
    EGUI_FontType font_type = EGUI_FontType_GUI;
    Rectangle shape_title   = GUI_WindowTitle(window->shape);
    Rectangle shape_panel   = GUI_WindowPanel(window->shape);
    Rectangle shape_bottom  = GUI_WindowBottom(window->shape);
    Rectangle workspace     = GUI_WindowWorkspace(window);
    Vector2 mouse           = GUI_CTX.temp->mouse_current;

    // Conditions
    bool is_window_target       = GUI_IsCurrentWindowTarget(window->id);
    bool is_pointer_overlay     = GUI_IsPointerOverOverlay();
    bool is_focusable           = is_window_target && GUI_CTX.temp->current_action == EGUI_WinActionNone && is_pointer_overlay == false;
    bool is_pointer_over        = CheckCollisionPointRec(mouse, window->shape);
    bool is_pointer_over_panel  = is_focusable && CheckCollisionPointRec(mouse, shape_panel);
    bool is_pointer_over_title  = is_focusable && CheckCollisionPointRec(mouse, shape_title) && !is_pointer_over_panel;
    bool is_pointer_over_bottom = is_focusable && CheckCollisionPointRec(mouse, shape_bottom);
    bool just_interacted        = is_pointer_over && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool is_z_priority          = GUI_CTX.state->z_index[0] == window->id;

    // Update pointer_over_gui
    if (is_pointer_over) GUI_CTX.temp->pointer_over_gui = true;

    // Focus?
    if (is_focusable && just_interacted && is_z_priority) {
        GUI_CTX.temp->current_action =  is_pointer_over_title   ? EGUI_WinActionMoving :
                                        is_pointer_over_bottom  ? EGUI_WinActionResizing
                                                                : EGUI_WinActionNone;
    }

    if (is_pointer_over_bottom || GUI_CTX.temp->current_action == EGUI_WinActionResizing) {
        GUI_CTX.temp->current_pointer = EGUI_Pointer_Resize;
    }

    // Active
    if (is_z_priority) {
        bool is_mouse_down = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        if (is_mouse_down == false) {
            GUI_CTX.temp->current_action = EGUI_WinActionNone;
        } else {
            // Movement
            if (GUI_CTX.temp->current_action == EGUI_WinActionMoving) {
                Vector2 mouse_current_valid     = LimitVector2Rect(GUI_CTX.temp->mouse_current, limits);
                Vector2 mouse_last_valid        = LimitVector2Rect(GUI_CTX.temp->mouse_last, limits);
                Vector2 displacement            = Vector2Subtract(mouse_current_valid, mouse_last_valid);

                window->shape.x += displacement.x;
                window->shape.y += displacement.y;
            }
            // Resizing
            if (GUI_CTX.temp->current_action == EGUI_WinActionResizing) {
                if (is_mouse_down) {
                    Vector2 mouse_valid     = LimitVector2Rect(GUI_CTX.temp->mouse_current, limits);
                    window->shape.width     = FloatMax(mouse_valid.x - window->shape.x, GUI_MIN_WIN_SIZE);
                    window->shape.height    = FloatMax(mouse_valid.y - window->shape.y, GUI_MIN_WIN_SIZE);
                }
            }
            // Handled by GUI, as move/resize can make that is_pointer_over is false during frames
            GUI_CTX.temp->pointer_over_gui = GUI_CTX.temp->current_action != EGUI_WinActionNone;
        }
    }

    // Limit
    window->shape   = LimitRect(window->shape, limits);
    shape_title     = GUI_WindowTitle(window->shape);

    // Vertical scroll
    bool horizontal_scroll  = workspace.height < window->content_height;
    if (horizontal_scroll) {
        if (is_pointer_over) {
            float wheel = GetMouseWheelMove();
            if (wheel != 0.0f) window->scroll_offset -= wheel * GUI_SCROLL_SPEED;
        }
        window->scroll_offset = Clamp(window->scroll_offset, 0, window->content_height - workspace.height);
    } else {
        window->scroll_offset = 0;
    }

    // Draw
    EGUI_ControlStatus status =
        is_z_priority           ? EGUI_ControlStatus_Focused :
        is_pointer_over_title   ? EGUI_ControlStatus_Collide :
                                  EGUI_ControlStatus_Default;
    GUI_DrawWindow(window, status, font_type);
    // Generate button panel
    GUI_WindowButtonPanel(window, font_type);
    GUI_WindowEndingPanel(window, font_type);
}


void GUI_CleanAndPrepareZIndex()
{
    GUI_State* state = GUI_CTX.state;

    // Remove unused window ids
    for (int j = 0; j < GUI_MAX_OPEN_WINS; ++j) {
        int id = state->z_index[j];
        if (id == 0)
            continue;

        bool found_id = false;
        for (int i = 0; i < GUI_MAX_OPEN_WINS; ++i) {
            GUI_Window* window = &state->window_s[i];
            if (window->id == id) {
                found_id = true;
                break;
            }
        }

        if (found_id == false) {
            state->z_index[j] = 0; // Clean
        }
    }

    // Grant that all window_s are in the z-index
    for (int i = 0; i < GUI_MAX_OPEN_WINS; i++) {
        GUI_Window* window = &state->window_s[i];
        if (window->id == 0)
            continue;

        int first_zero = -1;
        bool found_id = false;
        for (int j = 0; j < GUI_MAX_OPEN_WINS; ++j) {
            if (state->z_index[j] == window->id){
                found_id = true;
                break;
            }
            if (state->z_index[j] == 0 && first_zero < 0) {
                first_zero = j;
            }
        }
        if (found_id == false) {
            state->z_index[first_zero] = window->id;
        }
    }
}

void GUI_UpdateAndDrawWindows(Rectangle limits)
{
    GUI_State* state = GUI_CTX.state;

    GUI_CleanAndPrepareZIndex();

    // This variable allows to set force the z_index during the current frame
    bool is_pointer_overlay = GUI_IsPointerOverOverlay();
    bool force_z_index      = state->force_z_index > 0;
    bool interacting        = !force_z_index
                                && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
                                && is_pointer_overlay == false;

    if (interacting || force_z_index) {
        // Find ID
        int interacted_id       = state->force_z_index;
        bool forcing_z_index    = state->force_z_index > 0;
        // Restore state values -> z-index is being updated
        state->force_z_index = 0;

        int current_zindex  = -1;
        for (int j = 0; j < GUI_MAX_OPEN_WINS; ++j) {
            int id = state->z_index[j];
            if (id == 0) continue;

            GUI_Window* window = GUI_GetWindow(id);
            if (window == NULL) continue;

            bool find_window    = forcing_z_index && interacted_id == window->id;
            bool check_window   = !forcing_z_index && CheckCollisionPointRec(GUI_CTX.temp->mouse_current, window->shape);
            if (find_window || check_window) {
                interacted_id = window->id;
                current_zindex = j;
                break;
            }
        }

        if (interacted_id > 0 && current_zindex > 0) {
            // Move all elements to the right starting at current_zindex
            for (int j = current_zindex; j > 0; --j) {
                state->z_index[j] = state->z_index[j - 1];
            }

            // Add this one
            state->z_index[0] = interacted_id;
        }
    }

    // UPDATE WINDOW TARGET ID
    // Check collisions to determine current window_target_id (not only z-index priority but actual collision for this frame
    // you can be pointing to a 2nd window with a lower z-index priority.
    GUI_CTX.temp->window_target_id = 0;
    // If overlay is displayed then force window_target_id
    // This allows to click on the overlay when its in front of other window(s).
    if (GUI_CTX.temp->overlay_draw.window_target_id != 0) {
        GUI_CTX.temp->window_target_id = GUI_CTX.temp->overlay_draw.window_target_id;
    }
    // Normal windows
    if (GUI_CTX.temp->window_target_id  == 0) {
        for (int j = 0; j < GUI_MAX_OPEN_WINS; ++j) {
            int id = state->z_index[j];
            if (id == 0) continue;

            GUI_Window  *window         = GUI_GetWindow(id);
            bool        is_pointer_over = CheckCollisionPointRec(GUI_CTX.temp->mouse_current, window->shape);
            if (is_pointer_over) {
                GUI_CTX.temp->window_target_id = window->id;
                break;
            }
        }
    }

    // Process
    for (int j = GUI_MAX_OPEN_WINS - 1; j >= 0 ; --j) {
        int id = state->z_index[j];
        if (id == 0) continue;

        for (int i = 0; i < GUI_MAX_OPEN_WINS; i++) {
            GUI_Window* window = &state->window_s[i];
            if (window->id != id) continue;

            GUI_UpdateAndDrawWindow(window, limits);
            window->contents(window);
        }
    }
}

Rectangle GUI_BeginWindowContents(GUI_Window* window, EGUI_FontType font_type)
{
    // Grant min dimmensions
    window->shape.width = fmax(GUI_MIN_WIN_SIZE, window->shape.width);
    window->shape.height = fmax(GUI_MIN_WIN_SIZE, window->shape.height);

    // Data
    Rectangle window_workspace = GUI_WindowWorkspace(window);
    // Begin window stuff
    GUI_LayoutReset(window_workspace);

    // Vertical scroll
    GUI_CTX.temp->layout.current_window_idx         = window->id;
    GUI_CTX.temp->layout.current_window_workspace   = window_workspace;
    GUI_CTX.temp->layout.current_scroll             = -window->scroll_offset;
    GUI_CTX.temp->layout.current_font_type          = font_type;

    // Vertical scroll
    rlPushMatrix();
    rlTranslatef(0, -window->scroll_offset, 0);

    return window_workspace;
}

void GUI_EndWindowContents(GUI_Window* window)
{
    // End window stuff
    GUI_LayoutAutoJump();

    // Vertical scroll
    // Stored layout height
    window->content_height                   = GUI_CTX.temp->layout.used_height;
    // Reset temp values
    GUI_CTX.temp->layout                     = GUI_MakeLayoutTemp();

    // Finish draw instructions
    GUI_DrawOverlay();
    rlPopMatrix();
}

#endif
