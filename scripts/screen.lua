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

Screen = {}

local lgi = require 'lgi'
local cairo = lgi.cairo

function Screen:init()
  self.render_surface = cairo.ImageSurface.create(
    'ARGB32',
    xf.get_screen_width(),
    xf.get_screen_height()
  )
  self.cr = cairo.Context.create(self.render_surface)
end

function Screen:get_pixels()
  self.render_surface:flush()
  return self.render_surface:get_data()
end

function Screen:mark_dirty()
  self.render_surface:mark_dirty()
end

function Screen:clear()
  self.cr.operator = 'CLEAR'
  self.cr:paint()
end


return {Screen = Screen}

-- vi: et ts=2 sw=2

