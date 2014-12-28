# D2K Scripting Style Guide

This is the style guide for D2K scripting.  As I am still becoming familiar
with Lua, this is a work in progress, and as such it may change as time goes
on.

## General Guidelines

First, some general guidelines.

### Indenting

D2K uses an indent-width of 2 spaces.  **Never use tabs for any reason**.

### Tabs vs. Spaces

To repeat for emphasis: **never use tabs for any reason**.  Configure your
editor to convert tabs into 2 spaces.

### Line Width

Adhere strongly to 80-column buffer widths in your editor.  Yes it can be silly
and no we aren't on tiny terminals anymore, but having multiple editing buffers
open horizontally at a time (or browser windows, debuggers, etc.) is a much
bigger increase of productivity than not having to break a line.

### Spacing

  * Pad operators
    * Bad: `myargv[p+1]`
    * Good: `myargv[p + 1]`
    * Bad: `for i,w in pairs(self.widgets) do`
    * Good: `for i, w in pairs(self.widgets) do`
  * Do not pad parentheses
    * Bad: `xf.spawn_actor( x, y, z, xf.MT_PLAYER ) ;`
    * Good: `xf.spawn_actor(x, y, z, xf.MT_PLAYER);`
  * Pad function parameters
    * Bad: `xf.spawn_actor(x,y,z,MT_PLAYER);`
    * Good: `xf.spawn_actor(x, y, z, MT_PLAYER);`
  * Do not pad function names
    * Bad: `xf.spawn_actor (x, y, z, MT_PLAYER);`
    * Good: `xf.spawn_actor(x, y, z, MT_PLAYER);`

### Chunks, Blocks, etc.

Delimiters of chunks, blocks, control structures, and things delimited by Lua
keywords in general ought to be broken up.  For example, instead of:

    if error_occurred then error_and_exit('Error occurred') end

write this:

    if error_occurred then
      error_and_exit('Error occurred')
    end

Never put such things on a single line.

When passing around functions, only use anonymous functions for the **absolute
smallest** of functions.  When in doubt, define the function separately.

### Function Naming

Functions that are verbs ought to be named simply, i.e. `tick()` or `draw()`.
Functions that return booleans ought to be named with a form of be prefixing
them, for example, `was_updated()` or `is_dead()`.  Getters and setters should
be prefixed with `get_` and `set_`, as in `get_name()` and
`set_name('Doomguy')`.  Consistency in this regard is extremely important;
scripters ought to be able to guess most of your API by following these basic
rules.

