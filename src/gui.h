#ifndef UNITY_BUILD
 #include "gui_structs.h"
#endif

#define GUI_Assert(cond) \
    do { \
        if (!(cond)) { \
            fprintf(stderr, "GUI_Assert failed: %s, file %s, line %d\n", #cond, __FILE__, __LINE__); \
            exit(1); \
        } \
    } while (0)


// > POINTER
//   STABILITY : █████████░  90%
//   STATUS    : Stable
//   NOTES     : Nothing here
void GUI_DrawPointer()
{
    Vector2 mouse_current               = GUI_CTX.temp.mouse_current;
    GUI_PointerSetup* pointer_setup     = GUI_GetPointerSetup();
    Texture pointer_texture             = pointer_setup->pointer_texture;     

    Vector2 mouse_shape = (Vector2){
        mouse_current.x -
            (pointer_texture.width * pointer_setup->pointer_delta_normalized.x * pointer_setup->pointer_scale ),
        mouse_current.y - 
            (pointer_texture.height * pointer_setup->pointer_delta_normalized.y * pointer_setup->pointer_scale)
    };
    DrawTextureEx(pointer_texture, mouse_shape, 0, pointer_setup->pointer_scale , ColorAlpha(WHITE, pointer_setup->pointer_alpha));
}

bool GUI_WindowCollide(int window_id)
{
    return GUI_CTX.temp.window_coll_id != 0 && GUI_CTX.temp.window_coll_id != window_id;
}


bool GUI_CheckCollisionPointerWindow(int window_id, Rectangle shape)
{
    // This is not the window!
    if (GUI_WindowCollide(window_id)) {
        return false;
    }

    return CheckCollisionPointRec(GUI_CTX.temp.mouse_current, shape);
}

bool GUI_CheckCollisionPointerWindowTitle(int window_id, Rectangle shape_title)
{
    // This is not the window!... or we are moving
    if (GUI_WindowCollide(window_id) || GUI_CTX.temp.window_focus_moving) {
        return false;
    }

    return CheckCollisionPointRec(GUI_CTX.temp.mouse_current, shape_title);
}

bool GUI_CheckCollisionPointerControl(Rectangle shape, GUI_Window *window)
{
    GUI_State *state            = GUI_GetState();
    int focused_window_id       = state->z_index[0];
    Vector2 mouse               = GUI_CTX.temp.mouse_current;
    
    // Simple collision (outside a window)
    if (window == NULL || focused_window_id == 0) {
        return CheckCollisionPointRec(mouse, shape);
    }

    // This is not the window!
    if (GUI_WindowCollide(window->id)) {
        return false;
    }

    // Inside a window
    // Focused window data
    GUI_Window *focused_window  = GUI_GetWindow(focused_window_id);
    bool collide_focused        = CheckCollisionPointRec(mouse, focused_window->shape);

    // Vertical scroll data
    Vector2 current_scroll      = (Vector2) { 0, GUI_CTX.temp.current_scroll };
    bool collide_scolled        = CheckCollisionPointRec(mouse, MoveRect(shape, current_scroll));
    bool collide_workspace      = CheckCollisionPointRec(mouse, GUI_WindowWorkspace(window->shape, window->content_height));
    
    // Collide checks
    bool collide                = collide_scolled && collide_workspace;
    bool result                 = collide && (focused_window_id == window->id || !collide_focused);

    return result;
}


// > UTILS
//   STABILITY : ██████░░░░  60%
//   NOTES     : Nothing here
void GUI_DrawBorders(Rectangle shape, Color dark, Color light, float border, bool remove_corner)
{
    if (!remove_corner) {
        // Top
        DrawRectangle(shape.x, shape.y, shape.width, border, dark);
        // Left
        DrawRectangle(shape.x, shape.y, border, shape.height, dark);
        // Bottom
        DrawRectangle(shape.x, shape.y + shape.height - border, shape.width, border, light);
        // Right
        DrawRectangle(shape.x + shape.width - border, shape.y, border, shape.height, light);
    } else {
        // ─── Top (with corner gaps) ──────────────────────────────
        DrawRectangle(shape.x + border, shape.y, shape.width - 2 * border, border, dark);

        // │ Left (start after top gap)
        DrawRectangle(shape.x, shape.y + border, border, shape.height - border, dark);

        // │ Right (start after top gap)
        DrawRectangle(shape.x + shape.width - border, shape.y + border, border, shape.height - border, light);

        // └ Bottom (full width)
        DrawRectangle(shape.x, shape.y + shape.height - border, shape.width, border, light);
    }
}

void GUI_DrawAdjustedTextEx(const char* text, Vector2 position, Color tint, float scale, EGUI_FontType font_type)
{   
    GUI_FontSetup *setup    = GUI_GetFontSetup(font_type);
    GUI_State *state        = GUI_GetState();
    Font font               = GUI_GetFont(font_type);

    DrawTextEx(font, text, Vector2Add(position, Vector2Scale(setup->font_delta, state->scale)), font.baseSize * setup->font_scale * scale, setup->font_spacing, tint);
}

Vector2 GUI_MeasureAdjustedText(const char* text, EGUI_FontType font_type)
{
    GUI_FontSetup* setup    = &GUI_GetSetup()->font_setups[font_type];
    GUI_State* state        = GUI_GetState();
    Font font               = GUI_GetFont(font_type);
    
    Vector2 result = {
        MeasureTextEx(font, text, font.baseSize * setup->font_scale * state->scale, setup->font_spacing).x + setup->blink_delta.x * state->scale * setup->font_scale,
        MeasureTextEx(font, text, font.baseSize * setup->font_scale * state->scale, setup->font_spacing).y + setup->blink_delta.y * state->scale * setup->font_scale
    };

    return Vector2Add(result, Vector2Scale(setup->font_delta, state->scale));
}

void GUI_BeginDraw(EGUI_Pointer pointer_style)
{
    GUI_CTX.temp.current_pointer        = pointer_style;

    Rectangle mouse_limits = (Rectangle) {
        0,
        0,
        (float) GetScreenWidth(),
        (float) GetScreenHeight()
    };
    GUI_CTX.temp.mouse_current = LimitVector2Rect(GetMousePosition(), mouse_limits);        


    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        GUI_CTX.temp.control_focus_id = 0;
    }
}

void GUI_EndDraw()
{
    GUI_CTX.temp.mouse_last = GUI_CTX.temp.mouse_current;
}



// > ICON
//   STABILITY : █████████░  90%
//   NOTES     : Nothing here

// Returns used texture_scale to draw the texture in the available height
float GUI_Icon(Texture2D* texture2d, Vector2 position, float height, Color tint)
{
    GUI_Assert(height > 0);

    GUI_Setup *setup = GUI_GetSetup();
    
    float texture_scale = (height / texture2d->height);
    if (DEV_DEBUG_GUI) {
        DrawRectangleRec((Rectangle) { position.x, position.y, height, height }, ORANGE);
    }
    DrawTextureEx(*texture2d, Vector2Add(position, setup->icon_setup.icon_delta), 0, texture_scale, tint);
    return texture_scale;
}

float GUI_GetIconWidth()
{
    GUI_State *state = GUI_GetState();
    GUI_Setup *setup = GUI_GetSetup();
    return setup->icon_setup.icon_size * state->scale;
}


// > FACE
//   STABILITY : █████████░  90%
//   NOTES     : Nothing here
void GUI_DrawFace(Vector2 position, float height)
{
    GUI_Assert(height > 0);

    GUI_Icons *icons = GUI_GetIcons();
    Vector2 mouse = GUI_CTX.temp.mouse_current;

    // Center of the face
    Vector2 center = (Vector2){ position.x + height / 2.0f, position.y + height / 2.0f };

    // --- Distance-based color ---
    float dx = mouse.x - center.x;
    float dy = mouse.y - center.y;
    float dist = sqrtf(dx * dx + dy * dy);

    float max_dist = height * 4;

    // Clamp and normalizing
    float t = 1.0f - fminf(dist / max_dist, 1.0f);

    // Interpolation white - red
    Color color = (Color){
        255,
        (unsigned char)(255 * (1.0f - t)),
        (unsigned char)(255 * (1.0f - t)),
        255
    };
    float texture_scale = GUI_Icon(&icons->Face, position, height, color);
    GUI_Assert(texture_scale > 0);    

    // Determine quadrant relative to center
    int px = 0, py = 0;
    if (mouse.x >= center.x) px = 1; // right
    if (mouse.y >= center.y) py = 1; // bottom

    // Position of pixel (2x2 area)
    // float pixel_size = 2.0f * texture_scale;
    Vector2 pixel_pos = (Vector2){
        position.x + px * texture_scale,
        position.y + py * texture_scale
    };

    DrawRectangleV(Vector2Add((Vector2){ 4 * texture_scale, 6 * texture_scale }, pixel_pos), (Vector2){ texture_scale, texture_scale }, WHITE);
    DrawRectangleV(Vector2Add((Vector2){ 10 * texture_scale, 6 * texture_scale }, pixel_pos), (Vector2){ texture_scale, texture_scale }, WHITE);
}


// > BUTTON
//   STABILITY : █████████░  90%
//   NOTES     : Nothing here
void GUI_DrawButton(
    const char *text, Rectangle shape, Texture2D *icon,
    GUI_ElementStatus status, GUI_ThemeColors colors, EGUI_FontType font_type) 
{
    GUI_State *state            = GUI_GetState();
    GUI_FontSetup *font_setup   = GUI_GetFontSetup(font_type);
    GUI_Theme       *theme          = &GUI_CTX.setup->theme;

    float border    = font_setup->border;
    float scale     = state->scale;
    float color_change  = theme->color_change;
    float bg_alpha      = theme->bg_alpha;

    Color bg_color =    status == EGUI_Status_Focused  ? colors.bg_color_3 :
                        status == EGUI_Status_Focused  ? colors.bg_color_3 :
                        status == EGUI_Status_Collide  ? ColorBrightness(colors.bg_color_2, color_change) :
                                                         colors.bg_color_2;

    Color b_color_a =   status == EGUI_Status_Focused  ? colors.bg_color_3 :
                                                         colors.bg_color_1;

    Color b_color_b =   status == EGUI_Status_Focused  ? colors.bg_color_2 :
                                                         colors.bg_color_3;

    DrawRectangleRec(shape,  ColorAlpha(bg_color, bg_alpha));
    GUI_DrawBorders(shape, b_color_a, b_color_b, border * scale, false);

    float icon_w = icon == NULL ? 0 : GUI_GetIconWidth();

    GUI_DrawAdjustedTextEx(text, 
        (Vector2){ shape.x + icon_w + (border) * scale, shape.y + (border) * scale}, 
        colors.tx_color_0, scale, font_type);

    if (icon_w > 0) {
        GUI_Icon(icon, (Vector2) { shape.x + font_setup->border * state->scale, shape.y + font_setup->border * state->scale }, icon_w, WHITE);
    }
}

bool GUI_Button(
    const char* text, Rectangle shape, Texture2D* icon,
    GUI_ThemeColors colors)
{
    GUI_CONTROL_RELATIVE_TO_WINDOW(shape);
    GUI_CONTROL_FONT_TYPE_FROM_CONTEXT();

    GUI_ElementStatus status = EGUI_Status_Default;

    bool collide            = GUI_CheckCollisionPointerControl(shape, GUI_GetWindow(GUI_CTX.temp.current_window_idx));
    bool moving_window      = GUI_CTX.temp.window_focus_moving == 0;
    bool focusable          = collide && moving_window;
    if (focusable) {
        if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            status = EGUI_Status_Collide;
        } else {
            status = EGUI_Status_Focused;
        }
    }
    
    GUI_DrawButton(text, shape, icon, status, colors, font_type);
    
    return collide && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

// > LABEL
//   STABILITY : █████████░  90%
//   NOTES     : Nothing here
void GUI_DrawLabel(
    const char* text, Rectangle shape,
    GUI_ThemeColors colors, EGUI_FontType font_type)
{
    GUI_State *state            = GUI_GetState();
    GUI_FontSetup *font_setup   = GUI_GetFontSetup(font_type);

    float border    = font_setup->border;
    float scale     = state->scale;

    GUI_DrawAdjustedTextEx(text, 
        (Vector2){ shape.x + (border) * scale, shape.y + (border) * scale}, 
        colors.tx_color_0, scale, font_type);
}

void GUI_Label(const char* text, Rectangle shape, GUI_ThemeColors colors)
{
    GUI_CONTROL_RELATIVE_TO_WINDOW(shape);
    GUI_DrawLabel(text, shape, colors, GUI_CTX.temp.current_font_type);
}

// > TEXTBOX
//   STABILITY : █████████░  90%
//   NOTES     : Nothing here
void GUI_DrawTextBox(
    char* value, int *cursor, Rectangle shape,
    GUI_ElementStatus status, GUI_ThemeColors colors, bool blink, EGUI_FontType font_type)
{
    GUI_State       *state          = GUI_GetState();
    GUI_FontSetup   *font_setup     = GUI_GetFontSetup(font_type);
    GUI_Theme       *theme          = &GUI_CTX.setup->theme;

    float border        = font_setup->border;
    float scale         = state->scale;
    float color_change  = theme->color_change;
    float bg_alpha      = theme->bg_alpha;

    if (status == EGUI_Status_Default) 
        DrawRectangleRec(shape, ColorAlpha(colors.bg_color_1, bg_alpha));
    else if (status == EGUI_Status_Collide) 
        DrawRectangleRec(shape, ColorAlpha(ColorBrightness(colors.bg_color_1, color_change), bg_alpha));
    else if (status == EGUI_Status_Focused) 
        DrawRectangleRec(shape, ColorAlpha(ColorBrightness(colors.bg_color_1, -color_change), bg_alpha));    

    if (status == EGUI_Status_Focused) 
        GUI_DrawBorders(shape, ColorBrightness(colors.bg_color_2, -color_change), ColorBrightness(colors.bg_color_0, color_change), border * scale, false);
    else
        GUI_DrawBorders(shape, colors.bg_color_2, colors.bg_color_0, border * scale, false);

    GUI_DrawAdjustedTextEx(value, 
        (Vector2){ shape.x + (border) * scale, shape.y + (border) * scale}, 
        colors.tx_color_0, scale, font_type);

    if (status == EGUI_Status_Focused && blink) {
        Vector2 text_size = GUI_MeasureAdjustedText(value, font_type);
        
        char tmp[256] = {0};
        strncpy(tmp, value, *cursor);

        text_size = GUI_MeasureAdjustedText(tmp, font_type);
        DrawRectangle(
            shape.x + (border + font_setup->blink_delta.x) * scale + text_size.x,
            shape.y + (border + font_setup->blink_delta.y) * scale, 
            font_setup->blink_size.x * scale,
            font_setup->blink_size.y * scale, 
            ColorAlpha(colors.tx_color_0, font_setup->blink_alpha));
    }
}



void GUI_TextBox(
    int id, char *value, EGUI_InputType type,
    Rectangle shape, GUI_ThemeColors colors)
{
    GUI_CONTROL_RELATIVE_TO_WINDOW(shape)
    GUI_CONTROL_FONT_TYPE_FROM_CONTEXT()
    GUI_CONTROL_FOCUSED(id, shape, value)

    // Cursor per Id
    int *cursor = &GUI_CTX.state->textbox_cursors[id % GUI_MAX_TEXTBOXES];

    if (is_pointer_over) {
        GUI_CTX.temp.current_pointer = EGUI_Pointer_Text;
    }

    // Blink data
    const float blink_speed     = 0.5f;
    static float blink_timer    = 0.0f;
    static bool blink_state     = 0;

    // Gain focus
    if (just_focused) {
        // Activate blink cursor
        blink_state = 1;
        blink_timer = 0;

        // Re-locate cursor
        int textLength  = StringSize(value);
        int mouse_x = GUI_CTX.temp.mouse_current.x - shape.x;
        int cursor_position = 0;
        for (int i = 0; i <= textLength; i++) {
            cursor_position = i; 
            int w = GUI_MeasureAdjustedText(TextSubtext(value, 0, i), font_type).x;
            if (mouse_x < w) break;
        }
        *cursor = cursor_position;
    }

    // Focused
    if (is_focused) {
        int textLength = StringSize(value);

        // Handle text input
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126 && textLength < 255) { // Printable ASCII characters
                // Move chars to the right
                for (int i = textLength; i >= *cursor; i--) {
                    value[i + 1] = value[i];
                }
                value[*cursor] = (char)key;
                (*cursor)++;
                textLength++;
            }
            key = GetCharPressed();
        }

        // Erase
        if (IsKeyPressed(KEY_BACKSPACE) && *cursor > 0) {
            for (int i = *cursor - 1; i < textLength; i++) {
                value[i] = value[i+1];
            }
            (*cursor)--;
            textLength--;
        }
        if (IsKeyPressed(KEY_DELETE) && *cursor < textLength) {
            for (int i = *cursor; i < textLength; i++) {
                value[i] = value[i+1];
            }
            textLength--;
        }

        // Cursor movement
        if (IsKeyPressed(KEY_LEFT) && *cursor > 0)
            (*cursor)--;
        if (IsKeyPressed(KEY_RIGHT) && *cursor < textLength)
            (*cursor)++;
        if (IsKeyPressed(KEY_HOME))
            *cursor = 0;
        if (IsKeyPressed(KEY_END))
            *cursor = textLength;

        // Blink update
        if (blink_state)
            blink_timer += GetFrameTime();
        else
            blink_timer -= GetFrameTime();
        // Blink loop
        if (blink_timer > blink_speed)
            blink_state = 0;
        if (blink_timer < 0)
            blink_state = 1;
    }

    GUI_ElementStatus status =
        is_focused      ? EGUI_Status_Focused :
        is_pointer_over ? EGUI_Status_Collide :
                          EGUI_Status_Default;

    GUI_DrawTextBox(value, cursor, shape, status, colors, blink_state, font_type);
}



// > CHECKBOX
//   STABILITY : █████████░  90%
//   NOTES     : Improve draw
void GUI_DrawCheckBox(
    bool value, char *on_txt, char *off_txt, 
    Rectangle shape, GUI_ElementStatus status, 
    GUI_ThemeColors colors, EGUI_FontType font_type)
{
    GUI_State       *state          = GUI_GetState();
    GUI_FontSetup   *font_setup     = GUI_GetFontSetup(font_type);
    GUI_Theme       *theme          = &GUI_CTX.setup->theme;

    float border        = font_setup->border;
    float scale         = state->scale;
    float color_change  = theme->color_change;
    float bg_alpha      = theme->bg_alpha;

    Color tx = value ? colors.tx_color_0 : colors.bg_color_0;
    Color bg = value ? colors.bg_color_3 : colors.bg_color_2;
    Color b1 = value ? colors.bg_color_2 : colors.bg_color_0;
    Color b2 = value ? colors.bg_color_0 : colors.bg_color_2;
    if (status == EGUI_Status_Default) 
        DrawRectangleRec(shape, ColorAlpha(bg, bg_alpha));
    else if (status == EGUI_Status_Collide) 
        DrawRectangleRec(shape, ColorAlpha(ColorBrightness(bg, color_change), bg_alpha));
    else if (status == EGUI_Status_Focused) 
        DrawRectangleRec(shape, ColorAlpha(ColorBrightness(bg, -color_change), bg_alpha));
    

    if (status == EGUI_Status_Focused) 
        GUI_DrawBorders(shape, ColorBrightness(b1, -color_change), ColorBrightness(b2, color_change), border * scale, false);
    else
        GUI_DrawBorders(shape, b1, b2, border * scale, false);

    GUI_DrawAdjustedTextEx(value ? on_txt : off_txt,
        (Vector2){ shape.x + (border) * scale, shape.y + (border) * scale},
        tx, scale, EGUI_FontType_GUI);
}

void GUI_CheckBox(
    int id, bool *value, char *on_txt, char *off_txt, 
    Rectangle shape, GUI_ThemeColors colors)
{
    GUI_CONTROL_RELATIVE_TO_WINDOW(shape)
    GUI_CONTROL_FONT_TYPE_FROM_CONTEXT()
    GUI_CONTROL_FOCUSED(id, shape, value)

    // Focused
    if (is_focused) {
        if (is_pointer_active) {
            // Toggle checkbox
            *value = !(*value);
        }
    }

    GUI_ElementStatus status =
        is_focused      ? EGUI_Status_Focused :
        is_pointer_over ? EGUI_Status_Collide :
                          EGUI_Status_Default;
    GUI_DrawCheckBox(*value, on_txt, off_txt, shape, status, colors, font_type);
}


// > LAYOUT
//   STABILITY : █████████░  90%
//   NOTES     : Simplify default usage

#define RESET_COUNT     0
#define ADD_COUNT       1
#define ONLY_GET_COUNT  2
#define DEFAULT_SIZE    0.0

void GUI_BeginVertical(float size)
{
    GUI_CTX.temp.vertical_count = 0;
    GUI_CTX.temp.vertical_size  = size;
}
Rectangle GUI_NextVertical(void)
{
    float horizontal_size = GUI_CTX.temp.horizontal_size != DEFAULT_SIZE ? GUI_CTX.temp.horizontal_size 
                                                                    : (float)GetScreenWidth();
    float vertical_size = GUI_CTX.temp.vertical_size;

    Rectangle shape = {
        /* X */ horizontal_size * GUI_CTX.temp.horizontal_count,
        /* Y */ vertical_size * GUI_CTX.temp.vertical_count++,
        /* W */ horizontal_size,
        /* H */ vertical_size
    };
    return shape;
}
float GUI_GetAvailableHorizontal(Rectangle window_workspace)
{
    return window_workspace.width - (GUI_CTX.temp.horizontal_size * GUI_CTX.temp.horizontal_count);
}
void GUI_BeginHorizontal(float size)
{
    GUI_CTX.temp.horizontal_count = 0;
    GUI_CTX.temp.horizontal_size = size;
}
Rectangle GUI_NextHorizontal(void)
{
    float vertical_size = GUI_CTX.temp.vertical_size != DEFAULT_SIZE   ? GUI_CTX.temp.vertical_size 
                                                                        : (float)GetScreenHeight();
    float horizontal_size = GUI_CTX.temp.horizontal_size;

    Rectangle shape = { 
        /* X */ horizontal_size * GUI_CTX.temp.horizontal_count++,
        /* Y */ vertical_size * GUI_CTX.temp.vertical_count,
        /* W */ horizontal_size,
        /* H */ vertical_size
    };
    return shape;
}
Rectangle GUI_NextHorizontals(int quantity)
{
    GUI_Assert(quantity > 1);
    
    // Push value for next element
    Rectangle first = GUI_NextHorizontal();
    Rectangle last = {0};
    for (int i = 1; i < quantity; ++i) {
        last = GUI_NextHorizontal();
    }
    
    Rectangle result = {
        first.x,
        first.y,
        first.width + last.width,
        first.height
    };
    return result;
}
Rectangle GUI_NextVerticals(int quantity)
{
    GUI_Assert(quantity > 1);

    // Push value for next element
    Rectangle first = GUI_NextVertical();
    Rectangle last = {0};
    for (int i = 1; i < quantity; ++i) {
        last = GUI_NextVertical();
    }

    Rectangle result = {
        first.x,
        first.y,
        first.width,
        first.height + last.height
    };
    return result;
}

Rectangle GUI_WorkspaceAvailable(Rectangle workspace)
{
    float used_w = GUI_CTX.temp.horizontal_size * GUI_CTX.temp.horizontal_count;
    float used_h = GUI_CTX.temp.vertical_size   * GUI_CTX.temp.vertical_count;
    Rectangle result = {
        workspace.x + used_w,
        workspace.y + used_h,
        workspace.width - used_w,
        workspace.height - used_h
    };

    // Vertical scroll
    if (result.height < GUI_CTX.temp.vertical_size)
        result.height = GUI_CTX.temp.vertical_size;
    return result;
}
void GUI_ResetLayout()
{
    GUI_CTX.temp.horizontal_count = 0;
    GUI_CTX.temp.vertical_count   = 0;
}
void GUI_BeginBlock(float width, float height)
{
    // Add jump if necessary after ONLY horizontal blocks
    if (GUI_CTX.temp.horizontal_count > 0 && GUI_CTX.temp.vertical_count == 0) {
        GUI_NextVertical();
    }

    // Horizontal
    if (width > 0.0) {
        GUI_BeginHorizontal(width);
    } else if (width < 0.0) {
        GUI_BeginHorizontal(GUI_CTX.temp.current_workspace.width + width); // width is already negative
    } else {
        GUI_BeginHorizontal(GUI_CTX.temp.current_workspace.width);
    }

    // Adjust to get y-available space
    if (GUI_CTX.temp.vertical_count != 0) {
        GUI_CTX.temp.current_workspace = GUI_WorkspaceAvailable(GUI_CTX.temp.current_workspace);
    }

    // Vertical
    if (height > 0.0) {
        GUI_BeginVertical(height);
    } else if (height < 0.0) {
        GUI_BeginVertical(GUI_CTX.temp.current_workspace.height + height); // height is already negative
    } else {
        GUI_BeginVertical(GUI_CTX.temp.current_workspace.height);
    }
}
void GUI_BeginDuplicateBlock()
{
    GUI_BeginBlock(GUI_CTX.temp.horizontal_size, GUI_CTX.temp.vertical_size);
}

Rectangle GUI_Relative(Rectangle shape)
{
    GUI_CONTROL_RELATIVE_TO_WINDOW(shape);
    return shape;
}

// > WINDOW
//   STABILITY : █████████░  90%
//   NOTES     : Nothing here

void GUI_DrawWindow(GUI_Window* window,  GUI_ElementStatus status, EGUI_FontType font_type)
{
    GUI_State       *state          = GUI_GetState();
    GUI_FontSetup   *font_setup     = GUI_GetFontSetup(font_type);

    Rectangle        shape          = window->shape;
    Rectangle        shape_title    = GUI_WindowTitle(window->shape);
    GUI_ThemeColors  colors         = window->colors;
    GUI_Theme        *theme         = &GUI_CTX.setup->theme;

    float border        = font_setup->border;
    float scale         = state->scale;
    float color_change  = theme->color_change;
    float bg_alpha      = theme->bg_alpha;

    // Background
    DrawRectangleRec((Rectangle){shape.x + border * scale, shape.y + border * scale, shape.width - border * scale, shape.height - 2 * border * scale}, ColorAlpha(colors.bg_color_1, bg_alpha));
    GUI_DrawBorders(shape, colors.bg_color_0, colors.bg_color_2, border * scale, true);

    if (status == EGUI_Status_Default) {
        DrawRectangleRec(shape_title, ColorAlpha(colors.bg_color_2, bg_alpha));
        GUI_DrawBorders(shape_title, colors.bg_color_2, colors.bg_color_0, border * scale, false);
    } if (status == EGUI_Status_Focused) {
        DrawRectangleRec(shape_title, ColorAlpha(ColorBrightness(colors.bg_color_3, -color_change), bg_alpha));
        GUI_DrawBorders(shape_title, colors.bg_color_2, colors.bg_color_0, border * scale, false);
        GUI_DrawBorders(shape, colors.bg_color_0, ColorBrightness(colors.bg_color_3,-color_change), border * scale, true);
    } if (status == EGUI_Status_Collide) {
        DrawRectangleRec(shape_title, ColorAlpha(ColorBrightness(colors.bg_color_3, color_change), bg_alpha));
        GUI_DrawBorders(shape_title, colors.bg_color_2, colors.bg_color_0, border * scale, false);
        GUI_DrawBorders(shape, colors.bg_color_0, ColorBrightness(colors.bg_color_3, color_change), border * scale, true);
    }

    bool reserve_icon_space = window->icon != NULL || status == EGUI_Status_Focused;
    float icon_w = reserve_icon_space ? GUI_GetIconWidth() : 0;

    BeginScissorModeRect(AddRect(shape_title, 0, 0, -border * scale, -border * scale));
        GUI_DrawAdjustedTextEx(window->title,
            (Vector2) { shape_title.x + icon_w + (border) * scale, shape_title.y + (border) * scale }, 
            colors.tx_color_0, scale, EGUI_FontType_GUI);
    EndScissorMode();

    Vector2 icon_position = { shape_title.x + border * scale, shape_title.y + border * scale };
    if (window->icon != NULL && icon_w > 0) {
        GUI_Icon(window->icon, icon_position, icon_w, WHITE);
    }
    if (status == EGUI_Status_Focused && icon_w > 0) {
        GUI_DrawFace((Vector2) { shape_title.x + border * scale, shape_title.y + border * scale }, icon_w);
    }

    // Vertical scroll
    // Scrollbar
    Rectangle workspace = GUI_WindowWorkspace(shape, window->content_height);
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
}

void GUI_UpdateAndDrawWindow(GUI_Window *window, Rectangle limits)
{
    EGUI_FontType font_type = EGUI_FontType_GUI;
    Rectangle shape_title   = GUI_WindowTitle(window->shape);

    // Conditions
    bool is_pointer_over        = GUI_CheckCollisionPointerWindow(window->id, window->shape);
    bool is_pointer_over_title  = GUI_CheckCollisionPointerWindowTitle(window->id, shape_title);
    bool is_pointer_active      = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool just_focused           = is_pointer_over && is_pointer_active;
    bool is_focused             = GUI_CTX.state->z_index[0] == window->id;

    // Focus ?
    if (just_focused && GUI_CTX.temp.window_focus_moving == 0) {
        if (is_focused) {            
            GUI_CTX.temp.window_focus_moving = is_pointer_over_title;
        }
    }

    // Active
    if (is_focused){
        // Movement
        bool interacting        = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
        bool moving             = interacting && GUI_CTX.temp.window_focus_moving;
        if (moving) {
            Vector2 mouse_current_valid     = LimitVector2Rect(GUI_CTX.temp.mouse_current, limits);
            Vector2 mouse_last_valid        = LimitVector2Rect(GUI_CTX.temp.mouse_last, limits);
            Vector2 displacement            = Vector2Subtract(mouse_current_valid, mouse_last_valid);
            
            window->shape.x += displacement.x;
            window->shape.y += displacement.y;
        } else {
            GUI_CTX.temp.window_focus_moving = false;
        }
    }

    // Limit
    window->shape   = LimitRect(window->shape, limits);
    shape_title     = GUI_WindowTitle(window->shape);

    // Vertical scroll
    Rectangle workspace = GUI_WindowWorkspace(window->shape, window->content_height);
    if (window->content_height <= 0) {
        window->content_height = workspace.height;
    }
    if (is_pointer_over) {
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            window->scroll_offset -= wheel * GUI_SCROLL_SPEED;
            window->scroll_offset = Clamp(window->scroll_offset, 0, window->content_height - workspace.height);
        }
    }

    // Draw
    GUI_ElementStatus status = 
        is_focused              ? EGUI_Status_Focused :
        is_pointer_over_title   ? EGUI_Status_Collide :
                                  EGUI_Status_Default;
    GUI_DrawWindow(window, status, font_type);
}

GUI_Window* GUI_MakeWindow(
    int id, char *title, Rectangle shape, 
    GUI_ThemeColors colors, Texture2D *icon, 
    void (*contents)(GUI_Window*, void*))
{
    for (int i = 0; i < GUI_MAX_OPEN_WINS; ++i) {
        GUI_Window* window = &GUI_CTX.state->window_s[i];
        if (window->id == 0) {
            window->id          = id;
            window->shape       = shape;
            window->colors      = colors;
            window->title       = title;
            window->icon        = icon;
            window->contents    = contents;
            return window;
        }
    }
    return 0;
}

void GUI_UpdateAndDrawWindows(Rectangle limits, void* win_state)
{
    GUI_State* state = GUI_GetState();

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
    
    bool force_z_index  = state->force_z_index > 0;
    bool interacting    = !force_z_index && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    if (interacting || force_z_index) {
        // Find ID
        int interacted_id = state->force_z_index;
        int current_zindex = -1;

        // Restore state values z-index is being updated
        state->force_z_index = 0;

        for (int j = 0; j < GUI_MAX_OPEN_WINS; ++j) { 
            int id = state->z_index[j];
            if (id == 0) continue;

            GUI_Window* window = GUI_GetWindow(id);
            if (window == NULL) continue;

            bool find_window    = interacted_id > 0 && interacted_id == window->id;
            bool check_window   = interacted_id == 0 && CheckCollisionPointRec(GUI_CTX.temp.mouse_current, window->shape);
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

    // Check collisions
    GUI_CTX.temp.window_coll_id = 0;
    for (int j = 0; j < GUI_MAX_OPEN_WINS; ++j) { 
        int id = state->z_index[j];
        if (id == 0) continue;

        GUI_Window *window = GUI_GetWindow(id);
        if (GUI_CheckCollisionPointerWindow(window->id, window->shape)) {
            GUI_CTX.temp.window_coll_id = window->id;
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
            window->contents(window, win_state);
        }
    }
}

Rectangle GUI_BeginWindowContents(GUI_Window* window, float height, bool enable_scroll, EGUI_FontType font_type)
{
    // Data
    Rectangle window_workspace = GUI_WindowWorkspace(window->shape, height);
    
    // Vertical scroll    
    GUI_Assert(enable_scroll == false || height > window_workspace.height);
    window->content_height              = height;
    GUI_CTX.temp.current_window_idx     = window->id;
    GUI_CTX.temp.current_scroll         = -window->scroll_offset;
    GUI_CTX.temp.current_workspace      = window_workspace;
    GUI_CTX.temp.current_font_type      = font_type;
    
    // Begin window stuff
    GUI_ResetLayout();
    BeginScissorModeRect(window_workspace);
        // Vertical scroll    
        rlPushMatrix();
        rlTranslatef(0, -window->scroll_offset, 0);

    return window_workspace;
}

void GUI_EndWindowContents()
{
    // Close window stuff
        rlPopMatrix();
    EndScissorMode();

    // Vertical scroll
    // Reset
    GUI_CTX.temp.current_window_idx = GUI_NO_WIN;
    GUI_CTX.temp.current_scroll     = 0;
    GUI_CTX.temp.current_workspace  = (Rectangle){ 0, 0, 0, 0 };
    GUI_CTX.temp.current_font_type  = EGUI_FontType_Default;
}
