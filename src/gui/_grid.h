#pragma once
#ifndef UNITY_BUILD
#define IMPLEMENT_ALL   1
#include "main.h"
#endif


// > SUBMODULE: GRID
// > INDEX
GUI_GridTemp    GUI_MakeGrid();
float           GUI_GridHeightOrDefault();
float           GUI_GridWidthOrDefault();
Rectangle       GUI_GridRelative(Rectangle shape);
Rectangle       GUI_GridRelativePositionOnly(Rectangle shape);
// > IN PLACE QUERIES
Rectangle       GUI_GridAt(int x, int y);
Rectangle       GUI_GridBetween(int x, int y, int x_end, int Y_end);
// > GRID STARTERS
void            GUI_GridForX(float x_size);
void            GUI_GridForY(float y_size);
void            GUI_GridFor(float x, float y);
void            GUI_GridForCols(float cols, Rectangle window_workspace, EGUI_Font font);
void            GUI_GridDuplicate();
// > CONSUMABLES
Rectangle       GUI_GridNextX();
Rectangle       GUI_GridNextY();
Rectangle       GUI_GridNextXn(int n);
Rectangle       GUI_GridNextYn(int n);
// > INFO
Rectangle       GUI_GridAvailable(Rectangle workspace);
void            GUI_GridReset(Rectangle workspace);
void            GUI_GridAutoJump();

// > IMPLEMENTATION
#ifdef IMPLEMENT_ALL

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
    return GUI_CTX.temp->grid.vertical_size != DEFAULT_SIZE ? GUI_CTX.temp->grid.vertical_size
                                                            : (float)GetScreenHeight();
}

float GUI_GridWidthOrDefault()
{
    return GUI_CTX.temp->grid.horizontal_size != DEFAULT_SIZE ? GUI_CTX.temp->grid.horizontal_size
                                                              : (float)GetScreenWidth();
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

void GUI_GridHorizontal(float size)
{
    GUI_CTX.temp->grid.horizontal_count = 0;
    GUI_CTX.temp->grid.horizontal_size  = size;
}

void GUI_GridForY(float y_size)
{
    GUI_CTX.temp->grid.vertical_count = 0;
    GUI_CTX.temp->grid.vertical_size  = y_size;
}

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
    return result;
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
void GUI_GridReset(Rectangle workspace)
{
    GUI_CTX.temp->grid = GUI_MakeGrid();
    GUI_CTX.temp->grid.current_workspace      = workspace;
}
void GUI_GridAutoJump()
{
    bool used_space = GUI_CTX.temp->grid.horizontal_count > 0 && GUI_CTX.temp->grid.vertical_count == 0;
    if (used_space) {
        GUI_GridNextY();
    }
}
void GUI_GridSize(float width, float height)
{
    GUI_GridAutoJump();

    // Horizontal
    if (width > 0.0) {
        GUI_GridHorizontal(width);
    } else if (width < 0.0) {
        // width is already negative
        // so this takes available space minus width
        GUI_GridHorizontal(GUI_CTX.temp->grid.current_workspace.width + width);
    } else {
        GUI_GridHorizontal(GUI_CTX.temp->grid.current_workspace.width);
    }

    // Adjust to get y-available space
    if (GUI_CTX.temp->grid.vertical_count != 0) {
        GUI_CTX.temp->grid.current_workspace = GUI_GridAvailable(GUI_CTX.temp->grid.current_workspace);
    }

    // Vertical
    if (height > 0.0) {
        GUI_GridForY(height);
    } else if (height < 0.0) {
        // height is already negative
        // so this takes available space minus height
        float available  = GUI_CTX.temp->grid.current_workspace.height + height;
        if (available > 0) {
            GUI_GridForY(available);
        } else {
            GUI_GridForY(height * -1);
        }
    } else {
        GUI_GridForY(GUI_CTX.temp->grid.current_workspace.height);
    }
}

void GUI_GridForCols(float cols, Rectangle window_workspace, EGUI_Font font)
{
    float default_height = GUI_CalcDefaultHeightScaled(font);
    GUI_GridSize(window_workspace.width / cols, default_height);
    GUI_SetFontType(font);
}

void GUI_GridDuplicate()
{
    GUI_GridSize(GUI_CTX.temp->grid.horizontal_size, GUI_CTX.temp->grid.vertical_size);
}
#endif
