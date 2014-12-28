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

local lgi = require 'lgi'
local cairo = lgi.cairo
local pango = lgi.Pango
local hud_widget = require 'hud_widget'

Console = {}
setmetatable(Console, {__index = hud_widget.HUDWidget})

function Console:get_name()
  return 'console'
end

function Console:reset()
end

function Console:tick()
end

function Console:draw()
  local xc = 128
  local yc = 128
  local radius = 100
  local angle1 = math.rad(45)
  local angle2 = math.rad(180)

  self.hud.cr:set_source_rgba(0, 0, 0, 0.6)
  self.hud.cr.line_width = 10
  self.hud.cr:arc(xc, yc, radius, angle1, angle2)
  self.hud.cr:stroke()
  self.hud.cr:set_source_rgba(1, 0.2, 0.2, 0.6)
  self.hud.cr.line_width = 6
  self.hud.cr:arc(xc, yc, 10, 0, math.rad(360))
  self.hud.cr:fill()
  self.hud.cr:arc(xc, yc, radius, angle1, angle1)
  self.hud.cr:line_to(xc, yc)
  self.hud.cr:arc(xc, yc, radius, angle2, angle2)
  self.hud.cr:line_to(xc, yc)
  self.hud.cr:stroke()
end

function Console:was_updated()
  if self.should_update == nil then
    self.should_update = false
    return true
  end

  return true
end

return {Console = Console}

-- vi: et ts=2 sw=2

