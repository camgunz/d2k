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

-- local cairo = require('lgob.cairo')
-- local pango = require('lgob.pango')
-- local pangocairo = require('lgob.pangocairo')
local hud_widget = require('hud_widget')

Console = hud_widget.HUDWidget:new()

function Console:get_name()
  return 'console'
end

function Console:reset()
end

function Console:tick()
end

function Console:draw()
  local xc = 80
  local yc = 60
  local radius = 30
  local angle1 = math.rad(45)
  local angle2 = math.rad(180)

  d2k.overlay.context:set_source_rgba(0, 0, 0, 0.6)
  d2k.overlay.context:set_line_width(10)
  d2k.overlay.context:arc(xc, yc, radius, angle1, angle2)
  d2k.overlay.context:stroke()
  d2k.overlay.context:set_source_rgba(1, 0.2, 0.2, 0.6)
  d2k.overlay.context:set_line_width(6)
  d2k.overlay.context:arc(xc, yc, 10, 0, math.rad(360))
  d2k.overlay.context:fill()
  d2k.overlay.context:arc(xc, yc, radius, angle1, angle1)
  d2k.overlay.context:line_to(xc, yc)
  d2k.overlay.context:arc(xc, yc, radius, angle2, angle2)
  d2k.overlay.context:line_to(xc, yc)
  d2k.overlay.context:stroke()
end

return {Console = Console}

-- vi: et ts=2 sw=2

