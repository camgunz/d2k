
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

DEFAULT_FONT = 'sans serif'
DEFAULT_UNICODE_FALLBACK_FONTS = 'Arial Unicode MS,Unifont'
DEFAULT_FONT_SIZE = 15

Font = class('Font')

-- [CG] Currently, if you change the values of, say, `default_font` everything
--      that uses it will also change.  I think this isn't what I want, but
--      that means this current method of scaling won't work, so I'm leaving
--      it for now.

function Font:initialize(f)
    f = f or {}

    self.name = f.name or DEFAULT_FONT
    self.dec_size = f.size or DEFAULT_FONT_SIZE
end

function Font:get_name()
    return self.name
end

function Font:get_description()
    return string.format('%s,%s %spx',
        self:get_name(), DEFAULT_UNICODE_FALLBACK_FONTS, self:get_size()
    )
end

function Font:get_size()
    return math.floor(self.dec_size)
end

function Font:copy()
    return Font({name = self:get_name(), size = self.dec_size})
end

function Font:scale(factor)
    local old_dec_size = self.dec_size
    local old_size = self:get_size()

    self.dec_size = self.dec_size * factor
end

default_font = Font {
    name = 'sans serif',
    size = 15,
}

default_widget_font = Font {
    name = 'ascsys',
    size = 15,
}

default_hud_font = Font {
    name = 'ascsys',
    size = 15,
}

default_console_font = Font {
    name = 'ascsys',
    size = 15,
}

default_messages_font = Font {
    name = 'Zeroes Two',
    size = 15,
}

function get_default_font()
    return Font {
        name = default_font:get_name(),
        size = default_font:get_size()
    }
end

function get_default_widget_font()
    return Font {
        name = default_widget_font:get_name(),
        size = default_widget_font:get_size()
    }
end

function get_default_hud_font()
    return Font {
        name = default_hud_font:get_name(),
        size = default_hud_font:get_size()
    }
end

function get_default_console_font()
    return Font {
        name = default_console_font:get_name(),
        size = default_console_font:get_size()
    }
end

function get_default_messages_font()
    return Font {
        name = default_messages_font:get_name(),
        size = default_messages_font:get_size()
    }
end

return {
    get_default_font = get_default_font,
    get_default_widget_font = get_default_widget_font,
    get_default_hud_font = get_default_hud_font,
    get_default_console_font = get_default_console_font,
    get_default_messages_font = get_default_messages_font,
    scale_default_fonts = get_scale_default_fonts
}

-- vi: et ts=4 sw=4

