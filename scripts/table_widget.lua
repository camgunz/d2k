-------------------------------------------------------------------------------
--                                                                           --
-- D2K: A Doom Source Port for the 21st Century                              --
--                                                                           --
-- Copyright (C) 2014: See COPYRIGHT file                                    --
--                                                                           --
-- This file is part of D2K.                                                 --
--                                                                           --
-- D2K is free software: you can redistribute it and/or modify it under the  --
-- terms of the GNU General Public License as published by the Free Sofiware --
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
local TextWidget = require('text_widget')
local ContainerWidget = require('container_widget')

TableCell = class('TableCell', TextWidget.TextWidget)

function TableCell:initialize(tc)
  tc = tc or {}

  TextWidget.TextWidget.initialize(self, tc)

  self.header = false
  self.row = tc.row or 1
  self.column = tc.column or 1
end

function TableCell:is_header()
  return self.header
end

function TableCell:set_header(header)
  self.header = header
end

function TableCell:get_row()
  return self.row
end

function TableCell:set_row(row)
  self.row = row
end

function TableCell:get_column()
  return self.column
end

function TableCell:set_column(column)
  self.column = column
end

function TableCell:get_x()
  local x = TextWidget.TextWidget.get_x(self)

  for i=1,self.column - 1 do
    local cell = self.parent:get_cell(self.row, i)

    x = x + cell:get_pixel_width()
  end

  return x
end

function TableCell:get_y()
  local y = TextWidget.TextWidget.get_y(self)

  for i=1,self.row - 1 do
    local cell = self.parent:get_cell(i, self.column)

    y = y + cell:get_pixel_height()
  end

  return y
end

TableWidget = class('TableWidget', ContainerWidget.ContainerWidget)

function TableWidget:initialize(tw)
  tw = tw or {}

  self.rows = {}

  ContainerWidget.ContainerWidget.initialize(self, tw)

  self.cell_width = tw.cell_width or 1
  self.cell_height = tw.cell_height or 1
  self.cell_padding = tw.cell_padding or 0
  self.header_row_enabled = tw.header_row_enabled or false
  self.header_row_separator_enabled = header_row_separator_enabled or false
end

function TableWidget:update_cells_and_interfaces()
  self.interfaces = {}

  for i, row in ipairs(self.rows) do
    for j, cell in ipairs(row) do
      cell:set_column(j)
      cell:set_row(i)
      table.insert(self.interfaces, cell)
    end
  end
end

function TableWidget:is_header_row_enabled()
  return self.header_row_enabled
end

function TableWidget:set_header_row_enabled(header_row_enabled)
  self.header_row_enabled = header_row_enabled
end

function TableWidget:is_header_row_separator_enabled()
  return self.header_row_separator_enabled
end

function TableWidget:set_header_row_separator_enabled(separator_enabled)
  self.header_row_separator_enabled = separator_enabled
end

function TableWidget:get_cell(row, column)
  if row > #self.rows then
    return nil
  end

  if column > #self.rows[row] then
    return nil
  end

  return self.rows[row][column]
end

function TableWidget:add_row()
  self:insert_row(#self.rows + 1)
end

function TableWidget:insert_row(pos)
  local column_count = 0

  if #self.rows > 0 then
    column_count = #self.rows[1]
  end

  local row = {}

  for i=1,column_count do
    local table_cell = TableCell()

    table_cell:set_parent(self)
    table_cell:set_fg_color(self:get_fg_color())
    table_cell:set_bg_color(self:get_bg_color())
    table.insert(row, table_cell)
  end

  table.insert(self.rows, pos, row)

  self:update_cells_and_interfaces()
end

function TableWidget:remove_row(row)
  if row > 0 and row <= #self.rows then
    local table_row = self.rows[row]

    for i, table_cell in ipairs(table_row) do
      self:remove_interface(table_cell)
    end

    table.remove(self.rows, row)
  end

  self:update_cells_and_interfaces()
end

function TableWidget:add_column()
  local column_count = 0

  if #self.rows > 0 then
    column_count = #self.rows[1]
  end

  self:insert_column(column_count + 1)
end

function TableWidget:insert_column(column)
  for i, row in ipairs(self.rows) do
    local table_cell = TableCell()

    table_cell:set_parent(self)
    table_cell:set_fg_color(self:get_fg_color())
    table_cell:set_bg_color(self:get_bg_color())
    table.insert(row, column, table_cell)
  end

  self:update_cells_and_interfaces()
end

function TableWidget:remove_column(column)
  for i, row in ipairs(self.rows) do
    self:remove_interface(self.rows[row][column])
    table.remove(row, column)
  end

  self:update_cells_and_interfaces()
end

function TableWidget:set_cell_width(cell_width)
  for i, table_row in ipairs(self.rows) do
    for j, table_cell in ipairs(table_row) do
      table_cell:set_width(cell_width)
    end
  end
end

function TableWidget:set_cell_height(cell_height)
  for i, table_row in ipairs(self.rows) do
    for j, table_cell in ipairs(table_row) do
      table_cell:set_height(cell_height)
    end
  end
end

function TableWidget:set_column_width(column, column_width)
  for i, table_row in ipairs(self.rows) do
    for j, table_cell in ipairs(table_row) do
      if j == column then
        table_cell:set_width(column_width)
      end
    end
  end
end

function TableWidget:set_row_height(row, row_height)
  for i, table_row in ipairs(self.rows) do
    if i == row then
      for j, table_cell in ipairs(row) do
        table_cell:set_height(row_height)
      end
    end
  end
end

function TableWidget:get_row_count()
  return #self.rows
end

function TableWidget:get_column_count()
  if self:get_row_count() > 0 then
    return #self.rows[1]
  end

  return 0
end

function TableWidget:get_cell_count()
  local cell_count = 0

  for i, row in ipairs(self.rows) do
    for j, cell in ipairs(row) do
      cell_count = cell_count + 1
    end
  end

  return cell_count
end

return {
  TableCell   = TableCell,
  TableWidget = TableWidget
}

-- vi: et ts=2 sw=2

