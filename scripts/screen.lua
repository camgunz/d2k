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

Overlay = {}

local cairo = require('lgob.cairo')

function Overlay:new(s)
  s = s or {}
  setmetatable(s, self)
  self.__index = self

  return s
end

function Overlay:lock()
  self.surface = d2k.get_overlay_surface()
  self.context = d2k.get_overlay_context()
  d2k.lock_overlay()
end

function Overlay:unlock()
  d2k.unlock_overlay()
end

function Overlay:clear()
  self.context:set_operator(cairo.OPERATOR_CLEAR)
  self.context:paint()
end

return {Overlay = Overlay}

-- vi: et ts=2 sw=2

