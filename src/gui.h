
#ifndef UNITY_BUILD
 #define UNITY_BUILD 0
 #include "common.h"
 #include "gui_setup.h"
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

void GUI_DrawPointerFor(EGUI_Pointer pointer)
{
    GUI_PointerSetup* pointer_setup     = &GUI_CTX.setup->pointer_setups[pointer];
    Vector2 mouse_current               = GUI_CTX.temp->mouse_current;    
    Texture texture                     = pointer_setup->pointer_texture;
    Vector2 delta_normalized            = pointer_setup->pointer_delta_normalized;
    float scale                         = pointer_setup->pointer_scale * GUI_CTX.state->scale;

    Vector2 mouse_shape = (Vector2) {
        mouse_current.x - (texture.width * delta_normalized.x * scale),
        mouse_current.y - (texture.height * delta_normalized.y * scale)
    };
    DrawTextureEx(texture, mouse_shape, 0, scale, ColorAlpha(WHITE, pointer_setup->pointer_alpha));
}

void GUI_DrawPointer()
{
    EGUI_Pointer current_pointer        = GUI_CTX.temp->current_pointer;
    GUI_PointerSetup* pointer_setup     = &GUI_CTX.setup->pointer_setups[GUI_CTX.temp->current_pointer];
    if (pointer_setup->additional != EGUI_Pointer_None) {
        GUI_DrawPointerFor(pointer_setup->additional);
    }
    GUI_DrawPointerFor(current_pointer);
}

// raylib [shapes] example - Draw a mouse trail (position history)
void GUI_DrawPointerTrail()
{
    GUI_PointerSetup *pointer_setup = GUI_GetPointerSetup();
    Vector2 mouse                   = GUI_CTX.temp->mouse_current;

    // Shift all existing positions backward by one slot in the array
    // The last element (the oldest position) is dropped
    Vector2 *trail = GUI_CTX.temp->pointer_trail;
    for (int i = GUI_MAX_TRAIL - 1; i > 0; i--) {
        trail[i] = trail[i - 1];
    }
    Vector2 delta = (Vector2) {
        pointer_setup->trail_delta_normalized.x * pointer_setup->pointer_texture.width * pointer_setup->pointer_scale,
        pointer_setup->trail_delta_normalized.y * pointer_setup->pointer_texture.height * pointer_setup->pointer_scale
    };
    trail[0] = Vector2Add(mouse, delta);

    for (int i = 0; i < GUI_MAX_TRAIL; i++) {
        // Ensure we skip drawing if the array hasn't been fully filled on startup
        if ((trail[i].x != 0.0f) || (trail[i].y != 0.0f))
        {
            // Calculate relative trail strength (ratio is near 1.0 for new, near 0.0 for old)
            float ratio = (float)(GUI_MAX_TRAIL - i) / GUI_MAX_TRAIL; 
            
            // Fade effect: oldest positions are more transparent
            // Fade (color, alpha) - alpha is 0.5 to 1.0 based on ratio
            float alpha_min = 0.001f;
            float alpha_max = 0.05f;
            float trail_size = 5.0f;
            Color trail_color = Fade(WHITE, ratio * (alpha_max - alpha_min) + alpha_min);

            // Size effect: oldest positions are smaller
            float trail_radius = trail_size * ratio; 

            DrawCircleV(trail[i], trail_radius, trail_color);
        }
    }
}

bool GUI_CheckCollisionPointerControl(Rectangle shape, GUI_Window *window)
{
    GUI_State *state            = GUI_CTX.state;
    int focused_window_id       = state->z_index[0];
    Vector2 mouse               = GUI_CTX.temp->mouse_current;

    // Simple collision (outside a window)
    if (window == NULL || focused_window_id == 0) {
        return CheckCollisionPointRec(mouse, shape);
    }

    // This is not the window!
    if (GUI_IsCurrentWindowTarget(window->id) == false) {
        return false;
    }

    // Inside a window
    // Focused window data
    GUI_Window *focused_window  = GUI_GetWindow(focused_window_id);
    bool collide_focused        = CheckCollisionPointRec(mouse, focused_window->shape);

    // Vertical scroll data
    Vector2 current_scroll      = (Vector2) { 0, GUI_CTX.temp->layout.current_scroll };
    bool collide_scrolled       = CheckCollisionPointRec(mouse, MoveRect(shape, current_scroll));
    bool collide_workspace      = CheckCollisionPointRec(mouse, GUI_WindowWorkspace(window));
    bool overflow               = GUI_CTX.temp->layout.force_overflow;
    
    // Collide checks
    bool collide                = collide_scrolled && (collide_workspace || overflow);
    bool result                 = collide && (focused_window_id == window->id || !collide_focused);

    return result;
}

bool GUI_CheckCollisionPointerControlCurrentWin(Rectangle shape)
{
    return GUI_CheckCollisionPointerControl(shape, GUI_GetWindow(GUI_CTX.temp->layout.current_window_idx));
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
    GUI_State *state        = GUI_CTX.state;
    GUI_FontSetup *setup    = GUI_GetFontSetup(font_type);

    Font font               = GUI_GetFont(font_type);
    float font_scaled       = font.baseSize * setup->font_scale * scale;
    Vector2 delta_scaled    = Vector2Scale(setup->font_delta, state->scale);
    DrawTextEx(font, text, Vector2Add(position, delta_scaled), font_scaled, setup->font_spacing, tint);
}

Vector2 GUI_MeasureAdjustedText(const char* text, EGUI_FontType font_type)
{
    // Extract data
    GUI_State *state        = GUI_CTX.state;
    GUI_FontSetup* setup    = &GUI_GetSetup()->font_setups[font_type];
    Font font               = GUI_GetFont(font_type);

    // Process it
    float font_scaled       = font.baseSize * setup->font_scale * state->scale;
    Vector2 text_measure    = MeasureTextEx(font, text, font_scaled, setup->font_spacing);

    // Results (or statements)
    Vector2 result = {
        text_measure.x + setup->blink_delta.x * state->scale * setup->font_scale,
        text_measure.y + setup->blink_delta.y * state->scale * setup->font_scale
    };

    // Adjust
    Vector2 delta_scaled = Vector2Scale(setup->font_delta, state->scale);
    return Vector2Add(result, delta_scaled);
}

void GUI_BeginDraw(EGUI_Pointer pointer_style)
{
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

    // Button menus
    bool interacted         = IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || GetMouseWheelMove() != 0;
    bool just_interacted    = GUI_CTX.temp->buttonmenu_just_interacted;
    if (interacted && just_interacted == false) {
        //GUI_CTX.temp->buttonmenu_current = NULL;        
    }

    GUI_CTX.temp->buttonmenu_just_interacted   = false;
    GUI_CTX.temp->layout                       = GUI_MakeLayoutTemp();    
}

void GUI_DrawPendingButtonMenu()
{
    if (GUI_CTX.temp->buttonmenu_draw_function != NULL) {
        GUI_CTX.temp->buttonmenu_draw_function();
        GUI_CTX.temp->buttonmenu_draw_function  = NULL;
    }
}

void GUI_EndDraw()
{
    
    GUI_CTX.temp->mouse_last = GUI_CTX.temp->mouse_current;

    
    GUI_DrawPendingButtonMenu();
    
}

Rectangle GUI_ControlShapeCut(Rectangle shape, float border, float scale, bool intersect_window) {
    Rectangle result = AddRect(shape, border * scale, border * scale, -border * scale * 2, -border * scale * 2);

    result.y += GUI_CTX.temp->layout.current_scroll;
    if (intersect_window && GUI_CTX.temp->layout.current_window_idx != GUI_NO_WIN) {
        Rectangle intersection = RectIntersection(result, GUI_CTX.temp->layout.current_window_workspace);
        if (DEV_DEBUG_GUI_SCROLL) {
            if (GUI_CTX.temp->layout.current_window_idx == GUI_CTX.state->z_index[0]) {
                GUI_DrawBorders(GUI_CTX.temp->layout.current_window_workspace, RED, RED, 1, false);
                DrawDebugRect(result, ColorAlpha(GREEN, 0.1));
                DrawDebugRect(intersection, ColorAlpha(ORANGE, 0.9));
            }
        }
        result = intersection;
    }
    return result;
}

void GUI_BeginControlScissor()
{
    bool not_overflow   = GUI_CTX.temp->layout.force_overflow == false;
    bool inside_window  = GUI_CTX.temp->layout.current_window_idx != GUI_NO_WIN;
    if (not_overflow && inside_window) {
        BeginScissorModeRect(GUI_CTX.temp->layout.current_window_workspace);
    }
}

// Cut text not only by window but by the control itself
// Useful to cut text inside a control
void GUI_BeginInnerControlScissor(Rectangle shape, float border, float scale)
{
    bool not_overflow   = GUI_CTX.temp->layout.force_overflow == false;
    bool inside_window  = GUI_CTX.temp->layout.current_window_idx != GUI_NO_WIN;
    BeginScissorModeRect(GUI_ControlShapeCut(shape, border, scale, inside_window && not_overflow));
}


// > ICON
//   STABILITY : █████████░  90%
//   NOTES     : Nothing here

// Returns used texture_scale to draw the texture in the available height
float GUI_DrawIcon(Rectangle shape, Texture2D* texture2d, Color tint)
{
    GUI_Setup *setup    = GUI_CTX.setup;    
    float texture_scale = (shape.height / texture2d->height);

    GUI_BeginControlScissor();
        DrawTextureEx(*texture2d, Vector2Add((Vector2) { shape.x, shape.y }, setup->icon_setup.icon_delta), 0, texture_scale, tint);
    EndScissorMode();
    return texture_scale;
}

float GUI_Icon(Texture2D* texture2d, Vector2 position, float height, Color tint)
{
    GUI_Assert(height > 0);
    Rectangle shape = RectFromVector2(position, height, height);
    return GUI_DrawIcon(shape, texture2d, tint);
}

bool GUI_IconButton(Texture2D* texture2d, Vector2 position, float height, Color tint)
{
    Rectangle shape = RectFromVector2(position, height, height);
    GUI_MACRO_CONTROL_LAYOUT(shape);
    GUI_MACRO_CONTROL_ACTIVATED(shape);

    GUI_Theme *theme    = &GUI_CTX.setup->theme;
    float color_change  = theme->color_change;

    if (is_pointer_over)
        GUI_Icon(texture2d, position, height, tint);
    else
        GUI_Icon(texture2d, position, height, ColorBrightness(tint, -color_change));
    return is_active;
}


// > FACE
//   STABILITY : █████████░  90%
//   NOTES     : Nothing here
void GUI_Face(Vector2 position, float height)
{
    Rectangle shape = { position.x, position.y, height, height };
    GUI_MACRO_CONTROL_LAYOUT(shape);
    position.x = shape.x;
    position.y = shape.y;
    GUI_Assert(height > 0);

    GUI_Icons *icons = GUI_GetIcons();
    Vector2 mouse = GUI_CTX.temp->mouse_current;

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
    float texture_scale = GUI_DrawIcon(shape, &icons->Face, color);
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

// > IMAGE
//   STABILITY : █████████░  90%
//   NOTES     : Nothing here
void GUI_Image(Texture2D texture, Rectangle shape)
{
    GUI_MACRO_CONTROL_LAYOUT(shape);

    if (texture.id == 0) return;

    float tex_ratio = (float)texture.width / (float)texture.height;
    float dst_ratio = shape.width / shape.height;

    Rectangle src = { 0, 0, (float)texture.width, (float)texture.height };
    Rectangle dst = { 0 };

    if (dst_ratio > tex_ratio) {
        // limit height
        dst.height = shape.height;
        dst.width  = shape.height * tex_ratio;
        dst.x = shape.x + (shape.width - dst.width) * 0.5f;
        dst.y = shape.y;
    } else {
        // limit width
        dst.width  = shape.width;
        dst.height = shape.width / tex_ratio;
        dst.x = shape.x;
        dst.y = shape.y + (shape.height - dst.height) * 0.5f;
    }

    GUI_BeginControlScissor();
        DrawTexturePro(texture, src, dst, (Vector2){0,0}, 0.0f, WHITE);
    EndScissorMode();
}


// > BUTTON
//   STABILITY : █████████░  90%
//   NOTES     : Nothing here
void GUI_DrawButton(
    Rectangle shape, const char *text, Texture2D *icon,
    GUI_ElementStatus status, GUI_ThemeColors colors, EGUI_FontType font_type) 
{
    GUI_State *state            = GUI_CTX.state;
    GUI_FontSetup *font_setup   = GUI_GetFontSetup(font_type);
    GUI_Theme *theme            = &GUI_CTX.setup->theme;

    float border        = font_setup->border;
    float scale         = state->scale;
    float color_change  = theme->color_change;
    float bg_alpha      = theme->bg_alpha;
    float icon_w        = icon != NULL ? GUI_GetIconWidthForShape(shape, border) : 0;

    Color bg_color =    status == EGUI_Status_Focused  ? colors.bg_color_3 :
                        status == EGUI_Status_Focused  ? colors.bg_color_3 :
                        status == EGUI_Status_Collide  ? ColorBrightness(colors.bg_color_2, color_change) :
                                                         colors.bg_color_2;

    Color b_color_a =   status == EGUI_Status_Focused  ? colors.bg_color_3:
                                                         colors.bg_color_0;

    Color b_color_b =   status == EGUI_Status_Focused  ? colors.bg_color_2:
                                                         colors.bg_color_2;
    GUI_BeginControlScissor();
        DrawRectangleRec(shape,  ColorAlpha(bg_color, bg_alpha));
        GUI_DrawBorders(shape, b_color_a, b_color_b, border * scale, false);
    EndScissorMode();

    GUI_BeginInnerControlScissor(shape, border, scale);
        GUI_DrawAdjustedTextEx(text, 
            (Vector2){ shape.x + icon_w + (border) * scale, shape.y + (border) * scale}, 
            colors.tx_color_0, scale, font_type);

    if (icon_w > 0) {
        GUI_Icon(icon, (Vector2) { shape.x + font_setup->border * state->scale, shape.y + font_setup->border * state->scale }, icon_w, WHITE);
    }
    EndScissorMode();
}

bool GUI_Button(
    Rectangle shape, const char* text, Texture2D* icon,
    GUI_ThemeColors colors)
{
    GUI_MACRO_CONTROL_LAYOUT(shape);
    GUI_MACRO_CONTROL_ACTIVATED(shape);
    GUI_MACRO_CONTROL_FONT_TYPE_FROM_CONTEXT();

    GUI_ElementStatus status    = EGUI_Status_Default;
    if (is_pointer_over && is_activable) {
        if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            status = EGUI_Status_Collide;
        } else {
            status = EGUI_Status_Focused;
        }
    }
    GUI_DrawButton(shape, text, icon, status, colors, font_type);
    // TODO@dc: improve colors DrawDebugRect(shape, ColorAlpha(is_pointer_over? RED : BLUE, 0.2));
    return is_active;
}

bool GUI_ButtonMenu(
    Rectangle shape, const char* text, Texture2D* icon,
    GUI_ThemeColors colors, void (*draw_function)(void))
{
    bool was_active         = GUI_CTX.temp->buttonmenu_current == text;
    
    // Activate overflow when it was active
    if (was_active) {
        GUI_CTX.temp->layout.force_overflow = true;
    }
    
    bool is_active    = GUI_Button(shape, text, icon, colors);
    if (is_active) {
        if (!was_active) {
            GUI_CTX.temp->buttonmenu_current = text;
        } else {
            GUI_CTX.temp->buttonmenu_current = NULL;
        }
        GUI_CTX.temp->buttonmenu_just_interacted = true;
    }

    bool is_open = GUI_CTX.temp->buttonmenu_current == text;
    if (is_open) {
        GUI_CTX.temp->buttonmenu_draw_function  = draw_function;
        GUI_CTX.temp->buttonmenu_shape          = shape;
        GUI_CTX.temp->buttonmenu_layout         = GUI_CTX.temp->layout;
    }

    // Reset overrides (if any)
    GUI_CTX.temp->layout.force_overflow = false;
    return is_open;
}

// > TEXT
//   STABILITY : █████████░  90%
//   NOTES     : Nothing here
void GUI_DrawText(
    Rectangle shape, const char* text, 
    GUI_ThemeColors colors, EGUI_FontType font_type)
{
    GUI_State *state            = GUI_CTX.state;
    GUI_FontSetup *font_setup   = GUI_GetFontSetup(font_type);

    float border    = font_setup->border;
    float scale     = state->scale;

    GUI_BeginControlScissor();
        GUI_DrawAdjustedTextEx(text, 
            (Vector2){ shape.x + (border) * scale, shape.y + (border) * scale}, 
            colors.tx_color_0, scale, font_type);
    EndScissorMode();
}

void GUI_Text(Rectangle shape, const char* text, GUI_ThemeColors colors)
{
    GUI_MACRO_CONTROL_LAYOUT(shape);
    GUI_DrawText(shape, text, colors, GUI_CTX.temp->layout.current_font_type);
}

// > INPUT
//   STABILITY : █████████░  90%
//   NOTES     : Nothing here
void GUI_DrawInput(
    Rectangle shape, char* value, int blink_cursor,
    GUI_ElementStatus status, GUI_ThemeColors colors, bool blink, EGUI_FontType font_type)
{
    GUI_State       *state          = GUI_CTX.state;
    GUI_FontSetup   *font_setup     = GUI_GetFontSetup(font_type);
    GUI_Theme       *theme          = &GUI_CTX.setup->theme;

    float border        = font_setup->border;
    float scale         = state->scale;
    float color_change  = theme->color_change;
    float bg_alpha      = theme->bg_alpha;

    GUI_BeginControlScissor();
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
    EndScissorMode();

    // Auto horizontal scroll
    float auto_scroll_x = 0.0f;
    if (status == EGUI_Status_Focused) {
        float auto_scroll_right = 0.9f;
        float auto_scroll_left = 0.1f;
        Vector2 cursor_pos = {0};
        // TODO@dc: max text size
        char tmp[256] = {0};
        strncpy(tmp, value, blink_cursor);
        cursor_pos = GUI_MeasureAdjustedText(tmp, font_type);

        float visible_w = shape.width - (2.0f * border * scale);
        float cursor_x  = cursor_pos.x;

        // Right
        if (cursor_x - auto_scroll_x > visible_w * auto_scroll_right)
            auto_scroll_x = cursor_x - visible_w * auto_scroll_right;
        // Left
        else if (cursor_x - auto_scroll_x < visible_w * auto_scroll_left)
            auto_scroll_x = cursor_x - visible_w * auto_scroll_left;

        if (auto_scroll_x < 0) auto_scroll_x = 0;
    }

    GUI_BeginInnerControlScissor(shape, border, scale);
        GUI_DrawAdjustedTextEx(value, 
            (Vector2) { 
                shape.x + (border) * scale - auto_scroll_x,
                shape.y + (border) * scale
            }, colors.tx_color_0, scale, font_type);
    

        if (status == EGUI_Status_Focused && blink) {
            Vector2 text_size = GUI_MeasureAdjustedText(value, font_type);
            // TODO@dc: max text size
            char tmp[256] = {0};
            strncpy(tmp, value, blink_cursor);

            text_size = GUI_MeasureAdjustedText(tmp, font_type);
            DrawRectangle(
                shape.x + (border + font_setup->blink_delta.x) * scale + text_size.x - auto_scroll_x,
                shape.y + (border + font_setup->blink_delta.y) * scale, 
                font_setup->blink_size.x * scale,
                font_setup->blink_size.y * scale, 
                ColorAlpha(colors.tx_color_0, font_setup->blink_alpha));
        }
    EndScissorMode();
}

void GUI_Input(
    Rectangle shape, char *value,
    EGUI_InputType type, GUI_ThemeColors colors)
{
    GUI_MACRO_CONTROL_LAYOUT(shape)
    GUI_MACRO_CONTROL_FONT_TYPE_FROM_CONTEXT()
    GUI_MACRO_CONTROL_FOCUSED(value, shape)

    // Blink (text cursor)
    static void *last_control_focus_ptr = NULL;
    
    if (is_pointer_over) {
        GUI_CTX.temp->current_pointer = EGUI_Pointer_Text;
    }

    // Blink data
    const float     blink_speed     = 0.5f;
    static float    blink_timer     = 0.0f;
    static bool     blink_state     = 0;
    static int      blink_cursor    = 0;

    // Gain focus
    if (just_focused) {
        // Activate blink cursor
        blink_state = true;
        blink_timer = 0;

        // Re-locate cursor
        if (last_control_focus_ptr != value) {
            blink_cursor = 0;
        }
        last_control_focus_ptr = value;

        // TODO@dc: validate length
        int mouse_x     = GUI_CTX.temp->mouse_current.x - shape.x;
        int text_size   = StringSize(value);        
        bool right_test = mouse_x > GUI_MeasureAdjustedText(value, font_type).x;
        if (right_test) {
            blink_cursor = text_size;
        } else {
            for (blink_cursor = 0; blink_cursor < text_size; blink_cursor++) {
                int w = GUI_MeasureAdjustedText(TextSubtext(value, 0, blink_cursor), font_type).x;
                if (mouse_x < w) break;
            }
            blink_cursor = IntMax(0, --blink_cursor); // Move one position left for a more accurate mouse-to-text alignment
        }
    }

    // Focused
    if (is_focused) {
        // TODO@dc: validate length
        int text_size = StringSize(value);

        // Handle text input
        int key = GetCharPressed();
        while (key > 0) {
            // Special cases
            //
            if (type == EGUI_InputInt || type == EGUI_InputFloat) {
                if (key == '-') {
                    // Remove minus
                    if (value[0] == '-') {
                        int len = StringSize(value);
                        for (int i = 0; i < len; i++)
                            value[i] = value[i + 1];
                        blink_cursor--;
                        if (blink_cursor < 0) blink_cursor = 0;
                    // Add minus
                    } else {
                        int len = StringSize(value);
                        if (len < 254) {
                            for (int i = len; i >= 0; i--)
                                value[i + 1] = value[i];
                            value[0] = '-';
                            blink_cursor++;
                        }
                    }

                    // Already handled, skip.
                    key = GetCharPressed();
                    continue;
                }
            }

            // Default cases
            bool valid = false;
            switch (type) {
            case EGUI_InputText:
                valid = (key >= 32 && key <= 126);
                break;
            case EGUI_InputInt:
                if ((key >= '0' && key <= '9')) {
                    valid = true;
                }
                break;
            case EGUI_InputFloat:
                if ((key >= '0' && key <= '9') ||
                    (key == '.' && strchr(value, '.') == NULL)) {
                    valid = true;
                }
                break;
            }

            if (valid && text_size < 255) {
                // Move chars to the right
                for (int i = text_size; i >= blink_cursor; i--) {
                    value[i + 1] = value[i];
                }
                value[blink_cursor] = (char)key;
                (blink_cursor)++;
                text_size++;
            }
            key = GetCharPressed();
        }

        // Erase
        if (IsKeyPressed(KEY_BACKSPACE) && blink_cursor > 0) {
            for (int i = blink_cursor - 1; i < text_size; i++) {
                value[i] = value[i+1];
            }
            blink_cursor--;
            text_size--;
        }
        if (IsKeyPressed(KEY_DELETE) && blink_cursor < text_size) {
            for (int i = blink_cursor; i < text_size; i++) {
                value[i] = value[i+1];
            }
            text_size--;
        }

        // Cursor movement
        if (IsKeyPressed(KEY_LEFT) && blink_cursor > 0)
            blink_cursor--;
        if (IsKeyPressed(KEY_RIGHT) && blink_cursor < text_size)
            blink_cursor++;
        if (IsKeyPressed(KEY_HOME))
            blink_cursor = 0;
        if (IsKeyPressed(KEY_END))
            blink_cursor = text_size;

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

    GUI_DrawInput(shape, value, blink_cursor, status, colors, blink_state, font_type);
}



// > CHECKBOX
//   STABILITY : █████████░  90%
//   NOTES     : Improve draw
void GUI_DrawCheckBox(
    Rectangle shape, bool value, const char *on_txt, const char *off_txt, 
    GUI_ElementStatus status, GUI_ThemeColors colors, EGUI_FontType font_type)
{
    GUI_State       *state          = GUI_CTX.state;
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

    GUI_BeginControlScissor();
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
    EndScissorMode();

    GUI_BeginInnerControlScissor(shape, border, scale);
        GUI_DrawAdjustedTextEx(value ? on_txt : off_txt,
            (Vector2){ shape.x + (border) * scale, shape.y + (border) * scale},
            tx, scale, font_type);
    EndScissorMode();
}

void GUI_Check(
    Rectangle shape, bool *value, const char *on_txt, const char *off_txt, 
    GUI_ThemeColors colors)
{
    GUI_MACRO_CONTROL_LAYOUT(shape)
    GUI_MACRO_CONTROL_FONT_TYPE_FROM_CONTEXT()
    GUI_MACRO_CONTROL_FOCUSED(value, shape)

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
    GUI_DrawCheckBox(shape, *value, on_txt, off_txt, status, colors, font_type);
}


// > LAYOUT
//   STABILITY : █████████░  90%
//   NOTES     : Simplify default usage

#define RESET_COUNT     0
#define ADD_COUNT       1
#define ONLY_GET_COUNT  2
#define DEFAULT_SIZE    0.0

void GUI_LayoutVertical(float size)
{
    GUI_CTX.temp->layout.vertical_count = 0;
    GUI_CTX.temp->layout.vertical_size  = size;
}
float GUI_VerticalSizeOrDefault()
{
    return GUI_CTX.temp->layout.vertical_size != DEFAULT_SIZE   ? GUI_CTX.temp->layout.vertical_size
                                                        : (float)GetScreenHeight();
}
float GUI_HorizontalSizeOrDefault()
{
    return GUI_CTX.temp->layout.horizontal_size != DEFAULT_SIZE ? GUI_CTX.temp->layout.horizontal_size
                                                        : (float)GetScreenWidth();
}
Rectangle GUI_NextInPlace(int horizontal, int vertical)
{
    float horizontal_size   = GUI_HorizontalSizeOrDefault();
    float vertical_size     = GUI_CTX.temp->layout.vertical_size;

    Rectangle shape = {
        /* X */ horizontal_size * (GUI_CTX.temp->layout.horizontal_count + horizontal),
        /* Y */ vertical_size * (GUI_CTX.temp->layout.vertical_count + vertical),
        /* W */ horizontal_size,
        /* H */ vertical_size
    };
    return shape;
}
Rectangle GUI_NextVertical()
{
    Rectangle shape     = GUI_NextInPlace(0, 0);
    float vertical_size = GUI_CTX.temp->layout.vertical_size;

    GUI_CTX.temp->layout.used_height += vertical_size;
    GUI_CTX.temp->layout.vertical_count++;
    return shape;
}
float GUI_GetAvailableHorizontal(Rectangle window_workspace)
{
    return window_workspace.width - (GUI_CTX.temp->layout.horizontal_size * GUI_CTX.temp->layout.horizontal_count);
}
void GUI_LayoutHorizontal(float size)
{
    GUI_CTX.temp->layout.horizontal_count = 0;
    GUI_CTX.temp->layout.horizontal_size = size;
}
Rectangle GUI_NextHorizontal()
{
    Rectangle shape = GUI_NextInPlace(0, 0);
    GUI_CTX.temp->layout.horizontal_count++;
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
Rectangle GUI_LayoutAvailable(Rectangle workspace)
{
    float used_w = GUI_CTX.temp->layout.horizontal_size * GUI_CTX.temp->layout.horizontal_count;
    float used_h = GUI_CTX.temp->layout.vertical_size   * GUI_CTX.temp->layout.vertical_count;
    Rectangle result = {
        workspace.x + used_w,
        workspace.y + used_h,
        workspace.width - used_w,
        workspace.height - used_h
    };

    // Vertical scroll
    if (result.height < GUI_CTX.temp->layout.vertical_size)
        result.height = GUI_CTX.temp->layout.vertical_size;
    return result;
}
void GUI_LayoutReset(Rectangle workspace)
{
    GUI_CTX.temp->layout = GUI_MakeLayoutTemp();
    GUI_CTX.temp->layout.current_workspace      = workspace;
}
void GUI_LayoutAutoJump()
{
    // Add jump if necessary after ONLY horizontal blocks
    if (GUI_CTX.temp->layout.horizontal_count > 0 && GUI_CTX.temp->layout.vertical_count == 0) {
        GUI_NextVertical();
    }
}
void GUI_LayoutBlock(float width, float height)
{
    GUI_LayoutAutoJump();

    // Horizontal
    if (width > 0.0) {
        GUI_LayoutHorizontal(width);
    } else if (width < 0.0) {
        // width is already negative
        // so this takes avaiable space minus width
        GUI_LayoutHorizontal(GUI_CTX.temp->layout.current_workspace.width + width);
    } else {
        GUI_LayoutHorizontal(GUI_CTX.temp->layout.current_workspace.width);
    }

    // Adjust to get y-available space
    if (GUI_CTX.temp->layout.vertical_count != 0) {
        GUI_CTX.temp->layout.current_workspace = GUI_LayoutAvailable(GUI_CTX.temp->layout.current_workspace);
    }

    // Vertical
    if (height > 0.0) {
        GUI_LayoutVertical(height);
    } else if (height < 0.0) {
        // height is already negative
        // so this takes avaiable space minus height
        GUI_LayoutVertical(GUI_CTX.temp->layout.current_workspace.height + height);
    } else {
        GUI_LayoutVertical(GUI_CTX.temp->layout.current_workspace.height);
    }
}
void GUI_LayoutBlockCols(float cols, Rectangle window_workspace, EGUI_FontType font_type)
{
    float default_height = GUI_CalcDefaultHeightScaled(font_type);
    GUI_LayoutBlock(window_workspace.width / cols, default_height);
    GUI_FontType(font_type);
}
void GUI_LayoutDuplicateBlock()
{
    GUI_LayoutBlock(GUI_CTX.temp->layout.horizontal_size, GUI_CTX.temp->layout.vertical_size);
}
Rectangle GUI_Relative(Rectangle shape)
{
    GUI_MACRO_CONTROL_LAYOUT(shape);
    return shape;
}

// > WINDOW
//   STABILITY : █████████░  90%
//   NOTES     : Nothing here

void GUI_WindowButtonPanel(GUI_Window* window, EGUI_FontType font_type)
{
    GUI_Icons *icons            = GUI_GetIcons();
    GUI_FontSetup *font_setup   = GUI_GetFontSetup(font_type);
    GUI_ThemeColors colors      = window->colors;
    

    float border        = font_setup->border;
    float scale         = GUI_CTX.state->scale;
    float icon_sm_width = GUI_GetIconSmallWidth();

    Rectangle shape_panel   = GUI_WindowPanel(window->shape);
    Vector2 position_button = (Vector2) { shape_panel.x + border * scale, shape_panel.y };
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

void GUI_DrawWindow(GUI_Window* window,  GUI_ElementStatus status, EGUI_FontType font_type)
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

    bool reserve_icon_space = window->icon != NULL || (status == EGUI_Status_Focused && window->focused_face);
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
    if (status == EGUI_Status_Focused && icon_w > 0 && window->focused_face) {
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
    bool is_focusable           = is_window_target && GUI_CTX.temp->current_action == EGUI_ActionNone;
    bool is_pointer_over        = CheckCollisionPointRec(mouse, window->shape);
    bool is_pointer_over_panel  = is_focusable && CheckCollisionPointRec(mouse, shape_panel);
    bool is_pointer_over_title  = is_focusable && CheckCollisionPointRec(mouse, shape_title) && !is_pointer_over_panel;
    bool is_pointer_over_bottom = is_focusable && CheckCollisionPointRec(mouse, shape_bottom);
    bool just_interacted        = is_pointer_over && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
    bool is_z_priority          = GUI_CTX.state->z_index[0] == window->id;

    // Update pointer_over_gui
    if (is_pointer_over) GUI_CTX.temp->pointer_over_gui = true;

    // Focus ?
    if (is_focusable && just_interacted && is_z_priority) {            
        GUI_CTX.temp->current_action =   is_pointer_over_title   ? EGUI_ActionMoving :
                                        is_pointer_over_bottom  ? EGUI_ActionResizing
                                                                : EGUI_ActionNone;
    }

    if (is_pointer_over_bottom || GUI_CTX.temp->current_action == EGUI_ActionResizing) {
        GUI_CTX.temp->current_pointer = EGUI_Pointer_Resize;
    }

    // Active
    if (is_z_priority) {
        bool interacting = IsMouseButtonDown(MOUSE_BUTTON_LEFT);        
        if (interacting == false) {
            GUI_CTX.temp->current_action = EGUI_ActionNone;
        } else {
            // Movement
            if (GUI_CTX.temp->current_action == EGUI_ActionMoving) {
                Vector2 mouse_current_valid     = LimitVector2Rect(GUI_CTX.temp->mouse_current, limits);
                Vector2 mouse_last_valid        = LimitVector2Rect(GUI_CTX.temp->mouse_last, limits);
                Vector2 displacement            = Vector2Subtract(mouse_current_valid, mouse_last_valid);

                window->shape.x += displacement.x;
                window->shape.y += displacement.y;
            }
            // Resizing
            if (GUI_CTX.temp->current_action == EGUI_ActionResizing) {
                if (interacting) {
                    Vector2 mouse_valid     = LimitVector2Rect(GUI_CTX.temp->mouse_current, limits);
                    window->shape.width     = FloatMax(mouse_valid.x - window->shape.x, GUI_MIN_WIN_SIZE);
                    window->shape.height    = FloatMax(mouse_valid.y - window->shape.y, GUI_MIN_WIN_SIZE);
                }
            }
            // Handled by GUI, as move/resize can make that is_pointer_over is false during frames
            GUI_CTX.temp->pointer_over_gui = true;
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
    GUI_ElementStatus status = 
        is_z_priority           ? EGUI_Status_Focused :
        is_pointer_over_title   ? EGUI_Status_Collide :
                                  EGUI_Status_Default;
    GUI_DrawWindow(window, status, font_type);
    // Generate button panel
    GUI_WindowButtonPanel(window, font_type);
    GUI_WindowEndingPanel(window, font_type);
}

// TODO@dc: cleaup
void GUI_UpdateAndDrawWindows(Rectangle limits)
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
    
    // This variable allows to set force the z_index during the current frame
    bool force_z_index  = state->force_z_index > 0;
    bool interacting    = !force_z_index && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

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

    // Check collisions to determine current window_target_id (not only z-index priority but actual collision for this frame
    // you can be pointing to a 2nd window with a lower z-index priority.
    GUI_CTX.temp->window_target_id = 0;
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
    GUI_DrawPendingButtonMenu();
    rlPopMatrix();  
}
