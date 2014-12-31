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

local cairo = require('lgob.cairo')

function Screen:new(s)
  local scale_value = d2k.get_screen_multiply_value()

  s = s or {}
  setmetatable(s, self)
  self.__index = self

  s.render_surface = d2k.get_render_surface()
  s.cr = cairo.Context.create(s.render_surface)
  -- s.cr:scale(1, 1)

  return s
end

function Screen:clear()
  self.cr:set_operator(cairo.OPERATOR_CLEAR)
  self.cr:paint()
end


return {Screen = Screen}

-- vi: et ts=2 sw=2

