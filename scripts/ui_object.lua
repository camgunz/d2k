-------------------------------------------------------------------------------
--                                                                           --
-- D2K: A Doom Source Port for the 21st Century                              --
--                                                                           --
-- Copyright (C) 2014: See COPYRIGHT file                                    --
--                                                                           --
-- This file is part of D2K.                                                 --
--                                                                           --
-- D2K is free software: you can redistribute it and/or modify it under the  --
-- terms of the GNU General Public License as published by the Free Software --
-- Foundation, either version 2 of the License, or (at your option) any      --
-- later version.                                                            --
--                                                                           --
-- D2K is distributed in the hope that it will be useful, but WITHOUT ANY    --
-- WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS --
-- FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more    --
-- details.                                                                  --
--                                                                           --
-- You should have received a copy of the GNU General Public License along   --
-- with D2K.  If not, see <http://www.gnu.org/licenses/>.                    --
--                                                                           --
-------------------------------------------------------------------------------

local class = require('middleclass')
local lgi = require('lgi')
local Cairo = lgi.cairo
local Pango = lgi.Pango
local Fonts = require('fonts')

local REFERENCE_POINT_NORTH     = 0
local REFERENCE_POINT_NORTHEAST = 1
local REFERENCE_POINT_EAST      = 2
local REFERENCE_POINT_SOUTHEAST = 3
local REFERENCE_POINT_SOUTH     = 4
local REFERENCE_POINT_SOUTHWEST = 5
local REFERENCE_POINT_WEST      = 6
local REFERENCE_POINT_NORTHWEST = 7
local REFERENCE_POINT_CENTER    = 8

UIObject = class('UIObject')

function UIObject:initialize(uo)
    uo = uo or {}

    self.parent = nil
    self.active = false

    self.name                        = uo.name or 'UI Object'
    self.x                           = uo.x or 0
    self.y                           = uo.y or 0
    self.width                       = uo.width or 0
    self.height                      = uo.height or 0
    self.max_width                   = uo.max_width or 0
    self.max_height                  = uo.max_height or 0
    self.z_index                     = uo.z_index or 0
    self.top_padding                 = uo.top_padding or 0
    self.bottom_padding              = uo.bottom_padding or 0
    self.left_padding                = uo.left_padding or 0
    self.right_padding               = uo.right_padding or 0
    self.use_proportional_dimensions = uo.use_proportional_dimensions or true
    self.fg_color                    = uo.fg_color or {1.0, 1.0, 1.0, 1.0}
    self.bg_color                    = uo.bg_color or {0.0, 0.0, 0.0, 0.0}
    self.fullscreen                  = uo.fullscreen or false
    self.position_changed            = false
    self.dimensions_changed          = false
    self.cached_render               = nil
    self.local_reference_point       = uo.local_reference_point or
                                       REFERENCE_POINT_NORTHWEST
    self.parent_reference_point      = uo.parent_reference_point or
                                       REFERENCE_POINT_NORTHWEST

    if uo.font and uo.use_parent_font then
        error('Cannot specify a font while using parent font')
    end

    self.use_parent_font = uo.use_parent_font

    if not self.use_parent_font then
        if uo.font then
            self:set_font(uo.font)
        else
            self:set_font(Fonts.get_default_widget_font())
        end
    end
end

function UIObject:get_name()
    return self.name
end

function UIObject:set_name(name)
    self.name = name
end

function UIObject:get_local_reference_point()
    return self.local_reference_point
end

function UIObject:set_local_reference_point(local_reference_point)
    self.local_reference_point = local_reference_point
end

function UIObject:get_parent_reference_point()
    return self.parent_reference_point
end

function UIObject:set_parent_reference_point(parent_reference_point)
    self.parent_reference_point = parent_reference_point
end

function UIObject:get_x()
    local local_reference_point = self:get_local_reference_point()
    local parent_reference_point = self:get_parent_reference_point()
    local parent = self:get_parent()
    local x = 0

    if parent == nil then
        if parent_reference_point == REFERENCE_POINT_NORTH or
           parent_reference_point == REFERENCE_POINT_CENTER or
           parent_reference_point == REFERENCE_POINT_SOUTH then
            x = d2k.overlay:get_width() / 2
        elseif parent_reference_point == REFERENCE_POINT_NORTHEAST or
               parent_reference_point == REFERENCE_POINT_EAST or
               parent_reference_point == REFERENCE_POINT_SOUTHEAST then
            x = d2k.overlay:get_width()
        end
    elseif parent_reference_point == REFERENCE_POINT_NORTH or
           parent_reference_point == REFERENCE_POINT_CENTER or
           parent_reference_point == REFERENCE_POINT_SOUTH then
            x = (parent:get_x() + (self:get_parent_pixel_width() / 2))
    elseif parent_reference_point == REFERENCE_POINT_NORTHEAST or
           parent_reference_point == REFERENCE_POINT_EAST or
           parent_reference_point == REFERENCE_POINT_SOUTHEAST then
            x = (parent:get_x() + self:get_parent_pixel_width())
    else
        x = parent:get_x()
    end

    if local_reference_point == REFERENCE_POINT_NORTH or
       local_reference_point == REFERENCE_POINT_CENTER or
       local_reference_point == REFERENCE_POINT_SOUTH then
        x = x - ((
            self:get_left_padding() +
            self:get_pixel_width() +
            self:get_right_padding()
        ) / 2)
    elseif local_reference_point == REFERENCE_POINT_NORTHEAST or
           local_reference_point == REFERENCE_POINT_EAST or
           local_reference_point == REFERENCE_POINT_SOUTHEAST then
        x = x - (
            self:get_left_padding() +
            self:get_pixel_width() +
            self:get_right_padding()
        )
    else
    end

    x = x + self.x

    return x
end

function UIObject:set_x(x)
    self.x = x
    self:handle_position_change()
end

function UIObject:get_y()
    local local_reference_point = self:get_local_reference_point()
    local parent_reference_point = self:get_parent_reference_point()
    local parent = self:get_parent()
    local y = 0

    if parent == nil then
        if parent_reference_point == REFERENCE_POINT_SOUTHWEST or
           parent_reference_point == REFERENCE_POINT_SOUTH or
           parent_reference_point == REFERENCE_POINT_SOUTHEAST then
            y = d2k.overlay:get_height()
        elseif parent_reference_point == REFERENCE_POINT_WEST or
               parent_reference_point == REFERENCE_POINT_CENTER or
               parent_reference_point == REFERENCE_POINT_EAST then
            y = d2k.overlay:get_height() / 2
        end
    elseif parent_reference_point == REFERENCE_POINT_SOUTHWEST or
           parent_reference_point == REFERENCE_POINT_SOUTH or
           parent_reference_point == REFERENCE_POINT_SOUTHEAST then
            y = (parent:get_y() + self:get_parent_pixel_height())
    elseif parent_reference_point == REFERENCE_POINT_WEST or
           parent_reference_point == REFERENCE_POINT_CENTER or
           parent_reference_point == REFERENCE_POINT_EAST then
            y = (parent:get_y() + (self:get_parent_pixel_height() / 2))
    else
        y = parent:get_y()
    end

    if local_reference_point == REFERENCE_POINT_SOUTHWEST or
       local_reference_point == REFERENCE_POINT_SOUTH or
       local_reference_point == REFERENCE_POINT_SOUTHEAST then
        -- [CG] [FIXME] This commented-out block moves things too high, and
        --              I'm not entirely sure why

        -- y = y - ((
        --     self:get_top_padding() +
        --     self:get_pixel_height() +
        --     self:get_bottom_padding()
        -- ) / 2)

        y = y - self:get_pixel_height()
    elseif local_reference_point == REFERENCE_POINT_WEST or
           local_reference_point == REFERENCE_POINT_CENTER or
           local_reference_point == REFERENCE_POINT_EAST then
        y = y - ((
            self:get_top_padding() +
            self:get_pixel_height() +
            self:get_bottom_padding()
        ) / 2)
    end

    y = y + self.y

    return y
end

function UIObject:set_y(y)
    self.y = y
    self:handle_position_change()
end

function UIObject:get_width()
    if self.width < 0 or self.width > 1 then
        error(string.format('%s: Invalid width %s',
            self:get_name(), self.width
        ))
    end

    return self.width
end

function UIObject:set_width(width)
    if width == self.width then
        return
    end

    if not self:get_use_proportional_dimensions() then
        self.width = width
        self:handle_dimension_change()
        return
    end

    if width < 0 or width > 1 then
        error(string.format('%s: Invalid width %s', self:get_name(), width))
    end

    self.width = width
    self:handle_dimension_change()
end

function UIObject:get_pixel_width()
    if self:get_use_proportional_dimensions() then
        return math.floor(self.width * self:get_parent_pixel_width())
    end

    return self.width
end

function UIObject:set_pixel_width(width)
    if self:get_use_proportional_dimensions() then
        self.width = width / self:get_parent_max_pixel_width()
    else
        self.width = width
    end

    self:handle_dimension_change()
end

function UIObject:get_max_width()
    if not self:get_use_proportional_dimensions() then
        return self.max_width
    end

    if self.max_width < 0 or self.max_width > 1 then
        error(string.format('%s: Invalid max width %s',
            self:get_name(), self.max_width
        ))
    end

    if self.max_width == 0 then
        return self:get_parent_pixel_width()
    end

    return self:get_parent_pixel_width() * self.max_width
end

function UIObject:set_max_width(max_width)
    if max_width == self.max_width then
        return
    end

    if not self:get_use_proportional_dimensions() then
        self.max_width = max_width
        self:handle_dimension_change()
        return
    end

    if max_width < 0 or max_width > 1 then
        error(string.format('%s: Invalid max_width %s',
            self:get_name(), max_width
        ))
    end

    if max_width == 1 then
        self.max_width = self:get_parent_pixel_width()
    elseif max_width > 1 then
        self.max_width = max_width / self:get_parent_pixel_width()
    else
        self.max_width = max_width
    end

    self:handle_dimension_change()
end

function UIObject:get_max_pixel_width()
    if self:get_use_proportional_dimensions() then
        return self.max_width * self:get_parent_pixel_width()
    end

    return self.max_width
end

function UIObject:set_max_pixel_width(max_width)
    if self:get_use_proportional_dimensions() then
        self.max_width = max_width / self:get_parent_pixel_width()
    else
        self.max_width = max_width
    end

    self:handle_dimension_change()
end

function UIObject:get_height()
    if self.height < 0 or self.height > 1 then
        error(string.format('%s: Invalid height %s',
            self:get_name(), self.height
        ))
    end

    return self.height
end

function UIObject:get_parent_width()
    local parent = self:get_parent()

    if parent ~= nil and parent.get_width ~= nil then
        return parent:get_width()
    else
        return 1
    end
end

function UIObject:get_parent_max_width()
    local parent = self:get_parent()

    if parent ~= nil and parent.get_max_width ~= nil then
        return parent:get_max_width()
    else
        return 1
    end
end

function UIObject:get_parent_pixel_width()
    local parent = self:get_parent()

    if parent ~= nil and parent.get_pixel_width ~= nil then
        return parent:get_pixel_width()
    else
        return d2k.overlay:get_width()
    end
end

function UIObject:get_parent_max_pixel_width()
    local parent = self:get_parent()

    if parent ~= nil and parent.get_max_pixel_width ~= nil then
        return parent:get_max_pixel_width()
    else
        return d2k.overlay:get_width()
    end
end

function UIObject:set_height(height)
    if height == self.height then
        return
    end

    if not self:get_use_proportional_dimensions() then
        self.height = height
        self:handle_dimension_change()
        return
    end

    if height < 0 or height > 1 then
        error(string.format('%s: Invalid height %s',
            self:get_name(), height
        ))
    end

    self.height = height

    self:handle_dimension_change()
end

function UIObject:get_pixel_height()
    if self:get_use_proportional_dimensions() then
        return math.floor(self.height * self:get_parent_pixel_height())
    end

    return self.height
end

function UIObject:set_pixel_height(height)
    if self:get_use_proportional_dimensions() then
        local parent_max_pixel_height = self:get_parent_max_pixel_height()

        if parent_max_pixel_height == 0 then
            self.height = 0
        else
            self.height = height / self:get_parent_max_pixel_height()
        end
    else
        self.height = height
    end

    self:handle_dimension_change()
end

function UIObject:get_max_height()
    if not self:get_use_proportional_dimensions() then
        return self.max_height
    end

    if self.max_height < 0 or self.max_height > 1 then
        error(string.format('%s: Invalid max height %s',
            self:get_name(), self.max_height
      ))
    end

    if self.max_height == 0 then
        return self:get_parent_pixel_height()
    end

    return self:get_parent_pixel_height() * self.max_height
end

function UIObject:set_max_height(max_height)
    if max_height == self.max_height then
        return
    end

    if not self:get_use_proportional_dimensions() then
        self.max_height = max_height
        self:handle_dimension_change()
        return
    end

    if max_height < 0 or max_height > 1 then
        error(string.format('%s: Invalid max_height %s',
            self:get_name(), max_height
        ))
    end

    if max_height == 1 then
        self.max_height = d2k.overlay:get_height()
    elseif max_height > 1 then
        self.max_height = max_height / self:get_parent_pixel_height()
    else
        self.max_height = max_height
    end

    self:handle_dimension_change()
end

function UIObject:get_max_pixel_height()
    if self:get_use_proportional_dimensions() then
        return self.max_height * self:get_parent_pixel_height()
    end

    return self.max_height
end

function UIObject:set_max_pixel_height(max_height)
    if self:get_use_proportional_dimensions() then
        self.max_height = max_height / self:get_parent_pixel_height()
    else
        self.max_height = max_height
    end

    self:handle_dimension_change()
end

function UIObject:get_z_index()
    return self.z_index
end

function UIObject:get_top_padding()
    if not self:get_use_proportional_dimensions() then
        return self.top_padding
    end

    if self.top_padding < 0 or self.top_padding > 1 then
        error(string.format('%s: Invalid top padding %s',
            self:get_name(), self.top_padding
        ))
    end

    return self:get_parent_max_pixel_height() * self.top_padding
end

function UIObject:set_top_padding(top_padding)
    if top_padding == self.top_padding then
        return
    end

    if not self:get_use_proportional_dimensions() then
        self.top_padding = top_padding
        self:handle_dimension_change()
        return
    end

    if top_padding < 0 or top_padding > 1 then
        error(string.format('%s: Invalid top_padding %s',
            self:get_name(), top_padding
        ))
    end

    if top_padding == 1 then
        self.top_padding = self:get_parent_max_pixel_height()
    elseif top_padding > 1 then
        self.top_padding = top_padding / self:get_parent_max_pixel_height()
    else
        self.top_padding = top_padding
    end

    self:handle_dimension_change()
end

function UIObject:get_bottom_padding()
    if not self:get_use_proportional_dimensions() then
        return self.bottom_padding
    end

    if self.bottom_padding < 0 or self.bottom_padding > 1 then
        error(string.format('%s: Invalid bottom padding %s',
            self:get_name(), self.bottom_padding
        ))
    end

    return self:get_parent_max_pixel_height() * self.bottom_padding
end

function UIObject:set_bottom_padding(bottom_padding)
    if bottom_padding == self.bottom_padding then
        return
    end

    if not self:get_use_proportional_dimensions() then
        self.bottom_padding = bottom_padding
        self:handle_dimension_change()
        return
    end

    if bottom_padding < 0 or bottom_padding > 1 then
        error(string.format('%s: Invalid bottom_padding %s',
            self:get_name(), bottom_padding
        ))
    end

    if bottom_padding == 1 then
        self.bottom_padding = self:get_parent_max_pixel_height()
    elseif bottom_padding > 1 then
        self.bottom_padding = bottom_padding / self:get_parent_max_pixel_height()
    else
        self.bottom_padding = bottom_padding
    end

    self:handle_dimension_change()
end

function UIObject:get_left_padding()
    if not self:get_use_proportional_dimensions() then
        return self.left_padding
    end

    if self.left_padding < 0 or self.left_padding > 1 then
        error(string.format('%s: Invalid left padding %s',
            self:get_name(), self.left_padding
        ))
    end

    return self:get_parent_max_pixel_width() * self.left_padding
end

function UIObject:set_left_padding(left_padding)
    if left_padding == self.left_padding then
        return
    end

    if not self:get_use_proportional_dimensions() then
        self.left_padding = left_padding
        self:handle_dimension_change()
        return
    end

    if left_padding < 0 or left_padding > 1 then
        error(string.format('%s: Invalid left_padding %s',
            self:get_name(), left_padding
        ))
    end

    if left_padding == 1 then
        self.left_padding = self:get_parent_max_pixel_width()
    elseif left_padding > 1 then
        self.left_padding = left_padding / self:get_parent_max_pixel_width()
    else
        self.left_padding = left_padding
    end

    self:handle_dimension_change()
end

function UIObject:get_right_padding()
    if not self:get_use_proportional_dimensions() then
        return self.right_padding
    end

    if self.right_padding < 0 or self.right_padding > 1 then
        error(string.format('%s: Invalid right padding %s',
            self:get_name(), self.right_padding
        ))
    end

    return self:get_parent_max_pixel_width() * self.right_padding
end

function UIObject:set_right_padding(right_padding)
    if right_padding == self.right_padding then
        return
    end

    if not self:get_use_proportional_dimensions() then
        self.right_padding = right_padding
        self:handle_dimension_change()
        return
    end

    if right_padding < 0 or right_padding > 1 then
        error(string.format('%s: Invalid right_padding %s',
            self:get_name(), right_padding
        ))
    end

    if right_padding == 1 then
        self.right_padding = self:get_parent_max_pixel_width()
    elseif right_padding > 1 then
        self.right_padding = right_padding / self:get_parent_max_pixel_width()
    else
        self.right_padding = right_padding
    end

    self:handle_dimension_change()
end

function UIObject:get_parent_height()
    local parent = self:get_parent()

    if parent ~= nil and parent.get_height ~= nil then
        return parent:get_height()
    else
        return 1
    end
end

function UIObject:get_parent_max_height()
    local parent = self:get_parent()

    if parent ~= nil and parent.get_max_height ~= nil then
        return parent:get_max_height()
    else
        return 1
    end
end

function UIObject:get_parent_pixel_height()
    local parent = self:get_parent()

    if parent ~= nil and parent.get_pixel_height ~= nil then
        return parent:get_pixel_height()
    else
        return d2k.overlay:get_height()
    end
end

function UIObject:get_parent_max_pixel_height()
    local parent = self:get_parent()

    if parent ~= nil and parent.get_max_pixel_height ~= nil then
        return parent:get_max_pixel_height()
    else
        return d2k.overlay:get_height()
    end
end

function UIObject:set_z_index(z_index)
    local parent = self:get_parent()

    self.z_index = z_index

    if parent then
        parent:sort_interfaces()
    end

    self:handle_position_change()
end

function UIObject:get_use_proportional_dimensions()
    return self.use_proportional_dimensions
end

function UIObject:set_use_proportional_dimensions(pd)
    self.use_proportional_dimensions = pd
    self:handle_dimension_change()
end

function UIObject:get_screen_reference_point()
    return self.screen_reference_point
end

function UIObject:set_screen_reference_point(screen_reference_point)
    self.screen_reference_point = screen_reference_point
end

function UIObject:get_origin_reference_point()
    return self.origin_reference_point
end

function UIObject:set_origin_reference_point(origin_reference_point)
    self.origin_reference_point = origin_reference_point
end

function UIObject:get_fg_color()
    return self.fg_color
end

function UIObject:set_fg_color(fg_color)
    self.fg_color = fg_color
    self:handle_display_change()
end

function UIObject:get_bg_color()
    return self.bg_color
end

function UIObject:set_bg_color(bg_color)
    self.bg_color = bg_color
    self:handle_display_change()
end

function UIObject:is_fullscreen()
    return self.fullscreen
end

function UIObject:set_fullscreen(fullscreen)
    self.fullscreen = fullscreen

    self:handle_position_change()
    self:handle_dimension_change()
end

function UIObject:get_use_parent_font()
    return self.use_parent_font
end

function UIObject:set_use_parent_font(use_parent_font)
    self.use_parent_font = use_parent_font
    self:update_font_metrics()
end

function UIObject:get_font()
    if self.use_parent_font then
        if self.parent then
            return self.parent:get_font()
        end
    elseif self.font then
        return self.font
    end

    self.font = Fonts.get_default_widget_font()
    return self.font
end

function UIObject:set_font(font)
    if not font then
        font = Fonts.get_default_widget_font()
    end

    self.font = font
    self.use_parent_font = false

    self:update_font_metrics()
    self:handle_display_change()
end

function UIObject:get_font_ascent()
    if self.font_ascent == nil then
        self:update_font_metrics()
    end

    return self.font_ascent
end

function UIObject:get_font_descent()
    if self.font_descent == nil then
        self:update_font_metrics()
    end

    return self.font_descent
end

function UIObject:get_font_height()
    return self:get_font_ascent() + self:get_font_descent()
end

function UIObject:update_font_metrics()
    local font_metrics = d2k.overlay.text_context:get_metrics(
        Pango.FontDescription.from_string(self:get_font():get_description())
    )

    self.font_ascent = font_metrics:get_ascent() / Pango.SCALE
    self.font_descent = font_metrics:get_descent() / Pango.SCALE
end

function UIObject:get_cached_render()
    return self.cached_render
end

function UIObject:set_cached_render(render)
    self.cached_render = render
end

function UIObject:activate()
    local parent = self:get_parent()

    self.active = true

    if parent then
        parent:activate(self)
    end
end

function UIObject:deactivate()
    self.active = false
end

function UIObject:toggle()
    if self:is_active() then
        self:deactivate()
    else
        self:activate()
    end
end

function UIObject:is_active()
    return self.active
end

function UIObject:is_enabled()
    return self.parent ~= nil
end

function UIObject:get_parent()
    return self.parent
end

function UIObject:remove_parent()
    local current_parent = self:get_parent()

    if current_parent then
        self:get_parent():remove_interface(self)
        self.parent = nil
    end
end

function UIObject:set_parent(parent)
    self:remove_parent()

    if parent then
        parent:add_interface(self)
    end

    self.parent = parent
end

function UIObject:reset()
    self.position_changed = true
    self.dimensions_changed = true
end

function UIObject:tick()
end

function UIObject:invalidate_render()
    self.cached_render = nil
end

function UIObject:needs_rendering()
    if self.cached_render == nil then
        return true
    end

    return false
end

function UIObject:position_view()
end

function UIObject:clip_view()
    local cr = d2k.overlay.render_context

    cr:rectangle(
        self:get_x(),
        self:get_y(),
        self:get_pixel_width(),
        self:get_pixel_height()
    )
end

function UIObject:begin_render()
    d2k.overlay:lock()
    d2k.overlay.render_context:push_group_with_content(
        Cairo.Content.COLOR_ALPHA
    )
end

function UIObject:render()
end

function UIObject:end_render()
    d2k.overlay:unlock()
    self.cached_render = d2k.overlay.render_context:pop_group()
    self.position_changed = false
    self.dimensions_changed = false
    self.display_changed = false
end

function UIObject:get_render()
    if self:needs_rendering() then
        self:begin_render()
        self:render()
        self:end_render()
    end

    return self.cached_render
end

function UIObject:scale(value)
    self:get_font():scale(value)
end

function UIObject:handle_overlay_reset()
    self:reset()
    self:handle_position_change()
    self:handle_dimension_change()
    self:handle_display_change()
end

function UIObject:handle_position_change()
    self.position_changed = true
    self:invalidate_render()
end

function UIObject:handle_dimension_change()
    self.dimensions_changed = true
    self:invalidate_render()
end

function UIObject:handle_display_change()
    self.display_changed = true
    self:invalidate_render()
end

function UIObject:get_dimensions_changed()
    return self.dimensions_changed
end

return {
    UIObject                  = UIObject,
    REFERENCE_POINT_NORTH     = REFERENCE_POINT_NORTH,
    REFERENCE_POINT_NORTHEAST = REFERENCE_POINT_NORTHEAST,
    REFERENCE_POINT_EAST      = REFERENCE_POINT_EAST,
    REFERENCE_POINT_SOUTHEAST = REFERENCE_POINT_SOUTHEAST,
    REFERENCE_POINT_SOUTH     = REFERENCE_POINT_SOUTH,
    REFERENCE_POINT_SOUTHWEST = REFERENCE_POINT_SOUTHWEST,
    REFERENCE_POINT_WEST      = REFERENCE_POINT_WEST,
    REFERENCE_POINT_NORTHWEST = REFERENCE_POINT_NORTHWEST,
    REFERENCE_POINT_CENTER    = REFERENCE_POINT_CENTER
}

-- vi: et ts=4 sw=4
