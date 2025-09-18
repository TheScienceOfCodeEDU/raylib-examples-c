#ifndef UNITY_BUILD
 #include "rayext.h"
 #include "str.h"
 #include "env.h"
#endif

// Generates a color with a specified hue and interpolated saturation and value.
// Parameters:
//   hue: Hue value in degrees (e.g., 180.0f for cyan).
//   t: Interpolation factor [0.0, 1.0] controlling saturation (0.13 to 0.24) and value (0.89 to 0.33).
// Returns:
//   A Raylib Color struct with RGB values (0-255) and full alpha (255).
Color GetThemeColorFromHue(float hue, float t) {
    // Clamp t to [0, 1]
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    // Interpolate saturation (S) and value (V)
    // S ranges from 0.13 (light) to 0.24 (dark)
    // V ranges from 0.89 (light) to 0.33 (dark)
    float s = 0.13f + t * (0.24f - 0.13f);
    float v = 0.89f - t * (0.89f - 0.33f);
    s = s > 1.0f ? 1.0f : s;
    v = v > 1.0f ? 1.0f : v;

    // Convert HSV to RGB
    float c = v * s;            // Chroma
    float h_prime = hue / 60.0f; // Hue sector
    float x = c * (1.0f - fabsf(fmodf(h_prime, 2.0f) - 1.0f));
    float m = v - c;

    float r_prime, g_prime, b_prime;
    int sector = (int)h_prime;

    // Assign RGB based on hue sector
    switch (sector) {
        case 0: r_prime = c; g_prime = x; b_prime = 0; break;
        case 1: r_prime = x; g_prime = c; b_prime = 0; break;
        case 2: r_prime = 0; g_prime = c; b_prime = x; break;
        case 3: r_prime = 0; g_prime = x; b_prime = c; break;
        case 4: r_prime = x; g_prime = 0; b_prime = c; break;
        case 5: r_prime = c; g_prime = 0; b_prime = x; break;
        default: r_prime = 0; g_prime = 0; b_prime = 0; break;
    }

    // Scale to RGB (0-255) and create Raylib Color
    unsigned char r = (unsigned char)((r_prime + m) * 255.0f);
    unsigned char g = (unsigned char)((g_prime + m) * 255.0f);
    unsigned char b = (unsigned char)((b_prime + m) * 255.0f);

    return (Color){r, g, b, 255}; // Full alpha
}


struct {
    Texture2D New;
    Texture2D Open;
    Texture2D Save;
} typedef GUI_Icons;

GUI_Icons GUI_LoadIcons()
{
    GUI_Icons icons = {
        LoadTexture("ico/new.png"),
        LoadTexture("ico/open.png"),
        LoadTexture("ico/save.png")
    };
    return icons;
}

enum {
    GUI_Status_Default,
    GUI_Status_Focus,
    GUI_Status_Click
} typedef GUI_ElementStatus;

struct {
    Color tx_color;
    Color bg_color_0;
    Color bg_color_1;
    Color bg_color_2;
    Color bg_color_3;
} typedef GUI_ThemeColors;

GUI_ThemeColors Make_ThemeColors(float hue)
{
    GUI_ThemeColors colors = {
        GetThemeColorFromHue(hue, 0.0f),
        GetThemeColorFromHue(hue, 0.25f),
        GetThemeColorFromHue(hue, 0.5f),
        GetThemeColorFromHue(hue, 0.75f),
        GetThemeColorFromHue(hue, 1.0f)
    };
    return colors;
}

struct {
    Vector2 padding;        // Internal padding
    float border;           // Border thickness
    bool font_custom;       // Indicates if a custom font is used
    float font_spacing;     // Font spacing
    Vector2 blink_size;     // Size of the blinking cursor
    Vector2 blink_delta;    // Blink adjustment

    GUI_ThemeColors gray;
    GUI_ThemeColors red;
} typedef GUI_Theme;


GUI_Theme GUI_MakeDefaultTheme(unsigned char opacity)
{
    GUI_Theme theme = {
        /*// Background colors for another theme...
        (Color) { 80, 67, 48, opacity },    // bg_color_0: Dark brown with variable opacity
        (Color) { 116, 100, 67, opacity },  // bg_color_1: Medium brown with variable opacity
        (Color) { 58, 49, 35, opacity },    // bg_color_2: Very dark brown with variable opacity

        // Primary colors
        (Color) { 171, 158, 127, 255 },    // color_0: Light beige, fully opaque
        (Color) { 238, 208, 147, 255 },    // color_1: Warm beige, fully opaque
        (Color) { 253, 250, 85, 255 },     // color_2: Light yellow, fully opaque

        // Border colors
        (Color) { 33, 33, 33, 200 },       // b_color_0: Very dark gray, semi-opaque
        (Color) { 118, 118, 118, 200 },    // b_color_1: Medium gray, semi-opaque*/

        // Misc
        (Vector2) { 8, 8 },                // padding
        2.0f,                              // border
        1,                                 // font_custom
        1.0,                               // font_spacing
        (Vector2) { 2, 10 },               // blink_size
        (Vector2) { 1.9, 0 },              // blink_delta

        // Theme colors
        Make_ThemeColors(180.0f),
        Make_ThemeColors(3.0f),
    };

    return theme;
}

#define FOCUS_AVAILABLE -1
#define FOCUS_LOCKED    0

enum {
    GUI_Focus_Available,
    GUI_Focus_CanOverride,
    GUI_Focus_Granted
} typedef GUI_Focus;

struct {
    GUI_Theme   theme;
    GUI_Icons   icons;
    float       scale;
    Font        font_custom;

    int         window_focus_id;
    bool        window_focus_moving;
    int         control_focus_id;
    GUI_Focus   focus_state_current;

    Vector2     mouse_last;
    Vector2     mouse_current;

    float default_height;
} typedef GUI_State;

GUI_State GUI_MakeDefaultState(float opacity)
{
    GUI_State state = {
        GUI_MakeDefaultTheme(255),
        GUI_LoadIcons(),
        2.0f,
        LoadFont("fnt/pixelplay.png"),

        -1,
        0,
        0,
        GUI_Focus_Available,
        (Vector2){ 0.0, 0.0},
        (Vector2){ 0.0, 0.0},

        0
    };
    //SetTextureFilter(state.font.texture, TEXTURE_FILTER_POINT);
    return state;
}

Font GUI_GetFont(GUI_Theme theme, Font font_custom)
{
    Font font = theme.font_custom ? font_custom : GetFontDefault();
    return font;
}

float GUI_CalcDefaultHeight(GUI_State* gui)
{
    Font font = GUI_GetFont(gui->theme, gui->font_custom);
    Vector2 textShape = MeasureTextEx(GetFontDefault(), "Hello raylib", font.baseSize, gui->theme.font_spacing);
    return textShape.y;
}

float GUI_CalcShapeAvailableHeight(Rectangle shape, GUI_State* gui)
{
    return (shape.height - gui->theme.padding.y * 2) * gui->scale;
}

float GUI_CalcDefaultScaledHeight(GUI_State* gui)
{
    return (GUI_CalcDefaultHeight(gui) + gui->theme.border) * gui->scale + gui->theme.padding.y * 2;
}

float GUI_CalcDefaultIconSize(GUI_State* gui)
{
    return gui->default_height - gui->theme.border * 2 * gui->scale;
}


void GUI_DrawBorders(Rectangle shape, Color dark, Color light, float border)
{
    // Draw top border (horizontal line)    
    DrawRectangle(shape.x, shape.y, shape.width, border, dark);

    // Draw left border (vertical line)
    DrawRectangle(shape.x, shape.y, border, shape.height, dark);

    // Draw bottom border (horizontal line)
    DrawRectangle(shape.x, shape.y + shape.height - border, shape.width, border, light);

    // Draw right border (vertical line)
    DrawRectangle(shape.x + shape.width - border, shape.y, border, shape.height, light);
}

void GUI_DrawButton(char* text, Rectangle shape,  GUI_ElementStatus status, GUI_Theme theme, Font font_custom, GUI_ThemeColors colors, float scale, float icon_w) 
{
    Color bg_color =    status == GUI_Status_Click  ? colors.bg_color_3 : status == GUI_Status_Click  ? colors.bg_color_3 :
                        status == GUI_Status_Focus  ? ColorBrightness(colors.bg_color_2, 0.1)
                                                    : colors.bg_color_2;
    Color b_color_a =   status == GUI_Status_Click  ? colors.bg_color_3
                                                    : colors.bg_color_1;
    Color b_color_b =   status == GUI_Status_Click  ? colors.bg_color_2
                                                    : colors.bg_color_3;

    DrawRectangleRec(shape, bg_color);
    GUI_DrawBorders(shape, b_color_a, b_color_b, theme.border * scale);

    Font font = GUI_GetFont(theme, font_custom);
    DrawTextEx(font, text, 
        (Vector2){ shape.x + icon_w + theme.padding.x * 2 + theme.border * scale, shape.y + theme.padding.y + theme.border * scale}, 
        font.baseSize * scale, theme.font_spacing, colors.tx_color);
}

void GUI_Icon(Texture2D* texture2d, Vector2 position, float height, float scale, Color tint)
{
    scale *= height / texture2d->height;

    if (DEV_DEBUG_GUI) {
        DrawRectangleRec((Rectangle) { position.x, position.y, height, height }, ORANGE);
    }

    DrawTextureEx(*texture2d, position, 0, scale, tint);
}

bool GUI_Button(char* text, Rectangle shape, GUI_State* gui, Texture2D* icon, GUI_ThemeColors colors)
{
    GUI_Theme theme = gui->theme;
    GUI_ElementStatus status = GUI_Status_Default;

    bool collide            = CheckCollisionPointRec(gui->mouse_current, shape);
    bool moving_window      = gui->window_focus_moving == 0;
    bool focusable          = collide && moving_window;
    if (focusable) {
        if (!IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            status = GUI_Status_Focus;
        } else {
            status = GUI_Status_Click;
        }
    }
    
    float icon_w = GUI_CalcDefaultIconSize(gui);
    GUI_DrawButton(text, shape, status, theme, gui->font_custom, colors, gui->scale, icon_w);
    GUI_Icon(icon, 
        (Vector2) { shape.x + theme.border * gui->scale, shape.y + theme.border * gui->scale }, 
        icon_w, 1.0f, WHITE);

    return collide && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

void GUI_DrawTextBox(char* value, Rectangle shape,  GUI_ElementStatus status, GUI_Theme theme, Font font_custom, GUI_ThemeColors colors, float scale, bool blink)
{
    DrawRectangleRec(shape, colors.bg_color_3);
    GUI_DrawBorders(shape, colors.bg_color_2, colors.bg_color_0, theme.border * scale);

    Font font = GUI_GetFont(theme, font_custom);
    DrawTextEx(font, value, 
        (Vector2){ shape.x + theme.padding.x + theme.border * scale, shape.y + theme.padding.y + theme.border * scale}, 
        font.baseSize * scale, theme.font_spacing, colors.tx_color);

    if (status == GUI_Status_Focus && blink) {
        int text_w = MeasureTextEx(font, value, font.baseSize * scale, theme.font_spacing).x + theme.blink_delta.x * scale;
        int text_h = MeasureTextEx(font, value, font.baseSize * scale, theme.font_spacing).y + theme.blink_delta.y * scale;
        DrawRectangle(shape.x + theme.padding.x + text_w, shape.y + theme.padding.y, theme.blink_size.x * scale, text_h, 
            ColorAlpha(colors.tx_color, 0.5));
    }
}

void GUI_TextBox(int id, char* value, Rectangle shape, GUI_State* gui, GUI_ThemeColors colors)
{
    // Data
    GUI_Theme theme = gui->theme;

    // Blink
    const float blink_speed     = 0.5f;
    static float blink_timer    = 0.0f;
    static bool blink_state     = 0;

    // Conditions
    bool collide        = CheckCollisionPointRec(gui->mouse_current, shape);
    bool interacting    = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

    // Focus
    bool receives_focus = collide && interacting;
    if (receives_focus && gui->focus_state_current <= GUI_Focus_CanOverride) {
        gui->control_focus_id       = id;
        gui->focus_state_current    = GUI_Focus_Granted;
        blink_state                 = 1;
        blink_timer                 = 0;
    }
    bool focused                = gui->control_focus_id == id;
    GUI_ElementStatus status    = focused ? GUI_Status_Focus : GUI_Status_Default;

    // Update focused control
    if (focused) {
        int textLength = StringSize(value);
        // Handle text input
        int key = GetCharPressed();
        while (key > 0) {
            if (key >= 32 && key <= 126 && textLength < 255) { // Printable ASCII characters
                value[textLength] = (char)key;
                textLength++;
                value[textLength] = '\0'; // Null-terminate string
            }
            key = GetCharPressed(); // Get next queued character
        }

        // Handle backspace
        if (IsKeyPressed(KEY_BACKSPACE) && textLength > 0) {
            textLength--;
            value[textLength] = '\0';
        }

        // Update cursor blink timer
        if (blink_state)    blink_timer += GetFrameTime();
        else                blink_timer -= GetFrameTime();

        if (blink_timer > blink_speed)  blink_state = 0;
        if (blink_timer < 0)            blink_state = 1;
    }

    GUI_DrawTextBox(value, shape, status, gui->theme, gui->font_custom, colors, gui->scale, blink_state);
}

//
// Check box
//
void GUI_DrawCheckBox(bool value, char *on_txt, char *off_txt, Rectangle shape,  GUI_ElementStatus status, GUI_Theme theme, Font font_custom, GUI_ThemeColors colors, float scale)
{
    DrawRectangleRec(shape, colors.bg_color_3);
    GUI_DrawBorders(shape, colors.bg_color_2, colors.bg_color_0, theme.border * scale);

    Font font = GUI_GetFont(theme, font_custom);
    DrawTextEx(font, value ? on_txt : off_txt, 
        (Vector2){ shape.x + theme.padding.x + theme.border * scale, shape.y + theme.padding.y + theme.border * scale}, 
        font.baseSize * scale, theme.font_spacing, colors.tx_color);
}

void GUI_CheckBox(int id, bool *value, char *on_txt, char *off_txt, Rectangle shape, GUI_State* gui, GUI_ThemeColors colors)
{
    // Data
    GUI_Theme theme = gui->theme;

    // Conditions
    bool collide        = CheckCollisionPointRec(gui->mouse_current, shape);
    bool interacting    = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsKeyEnterPressed();

    // Focus
    bool receives_focus = collide && interacting;
    if (receives_focus && gui->focus_state_current <= GUI_Focus_CanOverride) {
        gui->control_focus_id       = id;
        gui->focus_state_current  = GUI_Focus_Granted;
    }
    bool focused                = gui->control_focus_id == id;
    GUI_ElementStatus status    = focused ? GUI_Status_Focus : GUI_Status_Default;

    // Update focused control
    if (focused) 
    {        
        if (interacting)
        {
             *value = !(*value); // Toggle the checkbox value
        }
    }

    GUI_DrawCheckBox(*value, on_txt, off_txt, shape, status, gui->theme, gui->font_custom, colors, gui->scale);
}

int GUI_GetVerticalCount(int value)
{
    static int count = 0;

    if (value == -1) {
        count = 0;
        return 0;
    } else {
        return count++;
    }
}

float GUI_GetVerticalSize(float value)
{
    static float size = 0;
    if (value != 0) size = value;
    return size;
}

Rectangle GUI_NextVertical()
{
    float size = GUI_GetVerticalSize(0.0);
    Rectangle textbox = { 0, size * GUI_GetVerticalCount(0), (float)GetScreenWidth(), size};
    return textbox;
}

void GUI_BeginVertical(float size)
{
    GUI_GetVerticalCount(-1);
    GUI_GetVerticalSize(size);
}

void GUI_DrawWindow(char* title, Rectangle shape, Rectangle shapeTitle,  GUI_ElementStatus status, GUI_Theme theme, Font font_custom, GUI_ThemeColors colors, float scale, bool icon)
{
    // Background
    DrawRectangleRec(shape, colors.bg_color_1);
    GUI_DrawBorders(shape, colors.bg_color_0, colors.bg_color_2, theme.border * scale);

    // Title
    DrawRectangleRec(shapeTitle, colors.bg_color_2);
    GUI_DrawBorders(shapeTitle, colors.bg_color_2, colors.bg_color_0, theme.border * scale);

    Font font = GUI_GetFont(theme, font_custom);
    DrawTextEx(font, title,
        (Vector2) { shapeTitle.x + theme.padding.x + theme.border * scale, shapeTitle.y + theme.padding.y + theme.border * scale }, 
        font.baseSize * scale, theme.font_spacing, colors.tx_color);
}

bool GUI_CheckCollisionPointRecWithMargin(Vector2 point, Rectangle rect, float margin) {
    Rectangle inner = {
        rect.x + margin,
        rect.y + margin, 
        rect.width - 2 * margin,
        rect.height - 2 * margin
    };

    return CheckCollisionPointRec(point, inner);
}

Rectangle GUI_WindowTitle(Rectangle shape, GUI_State* gui)
{
    Rectangle shapeTitle = {
        shape.x + gui->theme.border * gui->scale,
        shape.y + gui->theme.border * gui->scale,
        shape.width - 50,
        gui->default_height
    };
    return shapeTitle;
}

Rectangle GUI_WindowWorkspace(Rectangle shape, GUI_State* gui)
{
    Rectangle shape_title = GUI_WindowTitle(shape, gui);
    Rectangle shape_workspace = {
        shape_title.x,
        shape_title.y + shape_title.height + (shape_title.y - shape.y),
        shape.width - (shape_title.x - shape.x ) * 2,
        shape.height - shape_title.height - (shape_title.y - shape.y) - gui->theme.border * gui->scale * 2
    };

    if (DEV_DEBUG_GUI) {
        DrawRectangleRec(shape_title, ColorAlpha(ORANGE, 0.5));
        DrawRectangleRec(shape_workspace, ColorAlpha(GREEN, 0.5));
    }
    return shape_workspace;
}

void GUI_Window(int id, char* title, GUI_State* gui, Rectangle *shape,  Rectangle limits, GUI_ThemeColors colors)
{
    Rectangle shape_title = GUI_WindowTitle(*shape, gui);

    // Conditions
    bool collide        = CheckCollisionPointRec(gui->mouse_current, *shape);
    bool collide_title  = CheckCollisionPointRec(gui->mouse_current, shape_title);
    bool interacting    = IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    
    // Focus
    
    bool receives_focus     = collide && gui->window_focus_id == FOCUS_AVAILABLE;
    bool window_focusable   = gui->focus_state_current == GUI_Focus_Available;
    if (receives_focus && window_focusable) {
        gui->window_focus_id        = id;
        gui->window_focus_moving    = collide_title;
        gui->focus_state_current    = GUI_Focus_CanOverride; 
    }

    if (gui->window_focus_id == id){
        // Movement
        bool moving = interacting && gui->window_focus_moving;
        if (moving) {
            Vector2 mouse_current_valid     = LimitVector2Rect(gui->mouse_current, limits);
            Vector2 mouse_last_valid        = LimitVector2Rect(gui->mouse_last, limits);
            Vector2 displacement            = Vector2Subtract(mouse_current_valid, mouse_last_valid);
            
            shape->x += displacement.x;
            shape->y += displacement.y;
        } else {
            gui->window_focus_moving = false;
        }
    }

    // Limit
    *shape          = LimitRect(*shape, limits);
    shape_title     = GUI_WindowTitle(*shape, gui);

    // Draw
    GUI_ElementStatus status = gui->window_focus_id == id ? GUI_Status_Focus : GUI_Status_Default;
    GUI_DrawWindow(title, *shape, shape_title, status, gui->theme, gui->font_custom, colors, gui->scale, false);    
}
