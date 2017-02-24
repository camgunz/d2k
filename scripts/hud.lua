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
local Fonts = require('fonts')

local InputInterface = require('input_interface')
local ContainerWidget = require('container_widget')

local HUD = class('HUD', ContainerWidget.ContainerWidget)

local DEFAULT_X = 0
local DEFAULT_Y = 0
local DEFAULT_WIDTH  = 1
local DEFAULT_HEIGHT = 1
local DEFAULT_MAX_WIDTH  = 1
local DEFAULT_MAX_HEIGHT = 1
local DEFAULT_USE_PROPORTIONAL_DIMENSIONS = true

function HUD:initialize(h)
    h = h or {}

    h.name = h.name or 'HUD'
    h.font = h.font or Fonts.get_default_hud_font()
    h.x = h.x or DEFAULT_X
    h.y = h.y or DEFAULT_Y
    h.width = h.width or DEFAULT_WIDTH
    h.height = h.height or DEFAULT_HEIGHT
    h.max_width = h.max_width or DEFAULT_MAX_WIDTH
    h.max_height = h.max_height or DEFAULT_MAX_HEIGHT
    h.use_proportional_dimensions = h.use_proportional_dimensions or
                                    DEFAULT_USE_PROPORTIONAL_DIMENSIONS

    self.interfaces = {}

    ContainerWidget.ContainerWidget.initialize(self, h)
end

function HUD:sort_interfaces()
    table.sort(self.interfaces, function(i1, i2)
        return i1:get_z_index() < i2:get_z_index()
    end)
end

return {HUD = HUD}

-- vi: et ts=4 sw=4

