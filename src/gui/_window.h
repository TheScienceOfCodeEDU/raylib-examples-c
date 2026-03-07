#pragma once
#ifndef UNITY_BUILD
#define IMPLEMENT_ALL
#include "main.h"
#endif


// > SUBMODULE: WINDOW
// > INDEX
Rectangle       GUI_MakeWorkspace();
GUI_Window      GUI_MakeEmptyWindow(void);

// > WINDOW CONTROLS
//   SEE _controls.h > WINDOW

// > WINDOW RUNTIME

void            GUI_CleanAndPrepareZIndex();
void            GUI_UpdateAndDrawWindows(Rectangle limits);
Rectangle       GUI_BeginWindowContents(GUI_Window* window, EGUI_Font font);
void            GUI_EndWindowContents(GUI_Window* window);
// > WINDOW STATE
GUI_Window*     GUI_OpenWindow(
    int id, const char *title, GUI_ThemeColors colors,
    Texture2D *icon, bool focused_face, void (*contents)(GUI_Window*));
void            GUI_RemoveWindow(int id);

GUI_Window*     GUI_GetWindow(int id);
GUI_Window*     GUI_GetWindowByZindex(int z);
void            GUI_ForceZindex(int win_id);

Rectangle       GUI_GetWindowTitle(Rectangle shape);
Rectangle       GUI_GetWindowPanel(Rectangle shape);
Rectangle       GUI_GetWindowBottom(Rectangle shape);
void            GUI_WindowUpdateShapeForContent(GUI_Window *window);
Rectangle       GUI_GetWindowWorkspace(GUI_Window *window);

// > IMPLEMENTATION
#ifdef IMPLEMENT_ALL
Rectangle GUI_MakeWorkspace()
{
    Rectangle workspace = {
        0, 0, (float)GetScreenWidth(), (float)GetScreenHeight()
    };
    return workspace;
}

GUI_Window GUI_MakeEmptyWindow(void)
{
    GUI_Window window = {
        .id             = 0,
        .shape          = (Rectangle){ 0, 0, 0, 0 },
        .colors         = {{ 0 }},
        .title          = NULL,
        .icon           = NULL,
        .scroll_offset  = 0.0f,
        .focused_face   = true,

        .content_height = 0.0f,
        .contents       = NULL,
    };
    return window;
}

// > WINDOW
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
    GUI_State *state        = GUI_CTX.state;

    GUI_CleanAndPrepareZIndex();

    // This variable allows to set force the z_index during the current frame
    bool is_cursor_overlay = GUI_IsCursorOverOverlay();
    bool force_z_index      = state->force_z_index > 0;
    bool interacting        = !force_z_index
                                && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
                                && is_cursor_overlay == false;

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
            bool check_window   = !forcing_z_index && CheckCollisionPointRec(GUI_CTX.temp->cursor_current, window->shape);
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
    GUI_CTX.temp->window_target_id = GUI_NO_WIN;
    // If overlay is displayed then force window_target_id
    // This allows clicking on the overlay when its in front of other window(s).
    if (GUI_CTX.temp->overlay.window_target_id != GUI_NO_WIN) {
        GUI_CTX.temp->window_target_id = GUI_CTX.temp->overlay.window_target_id;
    }
    // Normal windows
    if (GUI_CTX.temp->window_target_id == GUI_NO_WIN) {
        for (int j = 0; j < GUI_MAX_OPEN_WINS; ++j) {
            int id = state->z_index[j];
            if (id == 0) continue;

            GUI_Window  *window         = GUI_GetWindow(id);
            bool        is_cursor_over  = CheckCollisionPointRec(GUI_CTX.temp->cursor_current, window->shape);
            if (is_cursor_over) {
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

            GUI_ProcessWindow(window, limits);
            window->contents(window);
        }
    }
}

Rectangle GUI_BeginWindowContents(GUI_Window* window, EGUI_Font font)
{
    // Grant min dimensions
    Rectangle min_size      =   GUI_MIN_WIN_RECT;
    window->shape.width     = FloatMax(min_size.width, window->shape.width);
    window->shape.height    = FloatMax(min_size.height, window->shape.height);

    // Data
    Rectangle window_workspace = GUI_GetWindowWorkspace(window);
    // Begin window stuff
    GUI_GridReset(window_workspace);
    GUI_SetFontType(font);

    // Set window data
    GUI_CTX.temp->window_current_idx              = window->id;
    GUI_CTX.temp->grid.current_window_workspace   = window_workspace;
    GUI_CTX.temp->grid.current_scroll             = -window->scroll_offset;

    // Vertical scroll
    rlPushMatrix();
    rlTranslatef(0, -window->scroll_offset, 0);

    return window_workspace;
}

void GUI_EndWindowContents(GUI_Window* window)
{
    // End window stuff
    GUI_GridAutoJump();

    // Vertical scroll
    // Stored grid height
    window->content_height = GUI_CTX.temp->grid.used_height;
    // Reset temp values
    GUI_CTX.temp->window_current_idx    = GUI_NO_WIN;
    GUI_CTX.temp->grid                  = GUI_MakeGrid();

    // Finish draw instructions
    GUI_AfterWindowContents();
    rlPopMatrix();
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
    int id, const char *title, GUI_ThemeColors colors,
    Texture2D *icon, bool focused_face, void (*contents)(GUI_Window*))
{
    GUI_Window *existing = GUI_GetWindow(id);
    if (existing)
        return existing;

    for (int i = 0; i < GUI_MAX_OPEN_WINS; ++i) {
        GUI_Window* window = &GUI_CTX.state->window_s[i];
        if (window->id == 0) {
            window->id              = id;
            window->shape           = GUI_MIN_WIN_RECT;
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
//   STABILITY: 90%
//   NOTES: Nothing here
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
#endif
