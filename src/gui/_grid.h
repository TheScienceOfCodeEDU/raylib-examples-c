#pragma once
#ifndef NON_EDITOR_BUILD
#define IMPLEMENT_ALL   1
#include "../common.h"
#endif


// > SUBMODULE: GRID
// > INDEX
// > BASICS
GUI_GridTemp    GUI_MakeGrid();
float           GUI_GridHeightOrDefault();
float           GUI_GridWidthOrDefault();
Rectangle       GUI_GridRelative(Rectangle shape);
Rectangle       GUI_GridRelativePositionOnly(Rectangle shape);
// > GRID STARTERS
void            GUI_GridReset(Rectangle workspace);
void            GUI_GridForX(float w);
void            GUI_GridForY(float h);
void            GUI_GridForXY(float w, float h);
void            GUI_GridFor(int columns, Rectangle window_workspace, EGUI_Font font);
void            GUI_GridForDuplicate();
// > IN PLACE QUERIES
Rectangle       GUI_GridAt(int x, int y);
Rectangle       GUI_GridBetween(int x, int y, int x_end, int y_end);
// > CONSUMABLES
Rectangle       GUI_GridNextX();
Rectangle       GUI_GridNextY();
Rectangle       GUI_GridNextXn(int n);
Rectangle       GUI_GridNextYn(int n);
void            GUI_GridAutoJump();
// > INFO
Rectangle       GUI_GridAvailable(Rectangle workspace);
Rectangle       GUI_GridApplyScroll(Rectangle shape);

// > IMPLEMENTATION
#ifdef IMPLEMENT_ALL

// > BASICS
GUI_GridTemp GUI_MakeGrid()
{
    GUI_GridTemp grid = {
        .current_workspace          = (Rectangle) { 0.f, 0.f, 0.f, 0.f},
        .vertical_count             = 0,
        .vertical_size              = 0.0f,
        .horizontal_count           = 0,
        .horizontal_size            = 0.0f,
        .used_height                = 0.0f,
        .current_scroll             = 0,
        .force_overflow             = false
    };
    return grid;
}

float GUI_GridHeightOrDefault()
{
    float vertical_size = GUI_CTX.temp->grid.vertical_size;
    return vertical_size != 0 ? vertical_size : (float)GetScreenHeight();
}

float GUI_GridWidthOrDefault()
{
    float horizontal_size = GUI_CTX.temp->grid.horizontal_size;
    return horizontal_size != 0 ? horizontal_size : (float)GetScreenWidth();
}

Rectangle GUI_GridRelative(Rectangle shape)
{
    bool is_active_grid = GUI_CTX.temp->grid.current_workspace.width  > 0 &&
                          GUI_CTX.temp->grid.current_workspace.height > 0;
    if (is_active_grid) {
        shape = RelativeToRect(shape, GUI_CTX.temp->grid.current_workspace);
    }
    return shape;
}

Rectangle GUI_GridRelativePositionOnly(Rectangle shape)
{
    Rectangle shape_relative    = GUI_GridRelative(shape);
    // Keep dimensions
    shape_relative.width        = shape.width;
    shape_relative.height       = shape.height;
    return shape_relative;
}
// < END BASICS

// > GRID STARTERS
void GUI_GridReset(Rectangle workspace)
{
    GUI_CTX.temp->grid                      = GUI_MakeGrid();
    GUI_CTX.temp->grid.current_workspace    = workspace;
}

void GUI_GridForX(float w)
{
    GUI_CTX.temp->grid.horizontal_count = 0;
    GUI_CTX.temp->grid.horizontal_size  = w;
}

void GUI_GridForY(float h)
{
    GUI_CTX.temp->grid.vertical_count = 0;
    GUI_CTX.temp->grid.vertical_size  = h;
}

void GUI_GridForXY(float w, float h)
{
    GUI_GridAutoJump();

    // Horizontal
    if (w > 0.0) {
        GUI_GridForX(w);
    } else if (w < 0.0) {
        // width is already negative
        // so this takes available space minus width
        GUI_GridForX(GUI_CTX.temp->grid.current_workspace.width + w);
    } else {
        GUI_GridForX(GUI_CTX.temp->grid.current_workspace.width);
    }

    // Adjust to get y-available space
    if (GUI_CTX.temp->grid.vertical_count != 0) {
        GUI_CTX.temp->grid.current_workspace = GUI_GridAvailable(GUI_CTX.temp->grid.current_workspace);
    }

    // Vertical
    if (h > 0.0) {
        GUI_GridForY(h);
    } else if (h < 0.0) {
        // height is already negative
        // so this takes available space minus height
        float available  = GUI_CTX.temp->grid.current_workspace.height + h;
        if (available > 0) {
            GUI_GridForY(available);
        } else {
            GUI_GridForY(h * -1);
        }
    } else {
        GUI_GridForY(GUI_CTX.temp->grid.current_workspace.height);
    }
}

void GUI_GridFor(int columns, Rectangle window_workspace, EGUI_Font font)
{
    float default_height = GUI_CalcDefaultHeightScaled(font);
    GUI_GridForXY(window_workspace.width / (float)columns, default_height);
    GUI_SetFont(font);
}

void GUI_GridForDuplicate()
{
    GUI_GridForXY(GUI_CTX.temp->grid.horizontal_size, GUI_CTX.temp->grid.vertical_size);
}
// < END GRID STARTERS

// > IN PLACE QUERIES
Rectangle GUI_GridAt(int x, int y)
{
    float horizontal_size   = GUI_GridWidthOrDefault();
    float vertical_size     = GUI_CTX.temp->grid.vertical_size;
    Rectangle result        = {
        .x      = horizontal_size * (float)(GUI_CTX.temp->grid.horizontal_count + x),
        .y      = vertical_size * (float)(GUI_CTX.temp->grid.vertical_count + y),
        .width  = horizontal_size,
        .height = vertical_size
    };
    return GUI_GridRelative(result);
}

Rectangle GUI_GridBetween(int x, int y, int x_end, int y_end)
{
    Rectangle begin     = GUI_GridAt(x, y);
    Rectangle end       = GUI_GridAt(x_end, y_end);
    Rectangle result    = {
        .x      = begin.x,
        .y      = begin.y,
        .width  = FloatAbs(end.x + end.width) - (begin.x),
        .height = FloatAbs(end.y + end.height) - (begin.y)
    };
    return result;
}
// < END IN PLACE QUERIES

// > CONSUMABLES
Rectangle GUI_GridNextX()
{
    Rectangle shape = GUI_GridAt(0, 0);
    GUI_CTX.temp->grid.horizontal_count++;
    return shape;
}

Rectangle GUI_GridNextY()
{
    Rectangle shape         = GUI_GridAt(0, 0);
    float vertical_size     = GUI_CTX.temp->grid.vertical_size;

    GUI_CTX.temp->grid.used_height += vertical_size;
    GUI_CTX.temp->grid.vertical_count++;
    return shape;
}

Rectangle GUI_GridNextXn(int n)
{
    Assert(n > 1);

    // Push value for next element
    Rectangle first = GUI_GridNextX();
    Rectangle last = {0};
    for (int i = 1; i < n; ++i) {
        last = GUI_GridNextX();
    }

    Rectangle result = {
        .x      = first.x,
        .y      = first.y,
        .width  = first.width + last.width,
        .height = first.height
    };
    return result;
}

Rectangle GUI_GridNextYn(int n)
{
    Assert(n > 1);

    // Push value for next element
    Rectangle first = GUI_GridNextY();
    Rectangle last = {0};
    for (int i = 1; i < n; ++i) {
        last = GUI_GridNextY();
    }

    Rectangle result = {
        .x      = first.x,
        .y      = first.y,
        .width  = first.width,
        .height = first.height + last.height
    };
    return result;
}

void GUI_GridAutoJump()
{
    bool used_space = GUI_CTX.temp->grid.horizontal_count > 0 && GUI_CTX.temp->grid.vertical_count == 0;
    if (used_space) {
        GUI_GridNextY();
    }
}
// < END CONSUMABLES

// > INFO
Rectangle GUI_GridAvailable(Rectangle workspace)
{
    float used_w = GUI_CTX.temp->grid.horizontal_size * (float)GUI_CTX.temp->grid.horizontal_count;
    float used_h = GUI_CTX.temp->grid.vertical_size   * (float)GUI_CTX.temp->grid.vertical_count;
    Rectangle result = {
        .x      = workspace.x + used_w,
        .y      = workspace.y + used_h,
        .width  = workspace.width - used_w,
        .height = workspace.height - used_h
    };

    // Vertical scroll
    if (result.height < GUI_CTX.temp->grid.vertical_size)
        result.height = GUI_CTX.temp->grid.vertical_size;
    return result;
}

Rectangle GUI_GridApplyScroll(Rectangle shape)
{
    Rectangle result = (Rectangle){
        .x      = shape.x,
        .y      = shape.y /*- GUI_CTX.temp->grid.current_scroll*/,
        .width  =  shape.width,
        .height = shape.height
    };
    return result;
}
// < END INFO
#endif
