# D2K Style Guide

Code style is important; it improves readability thus enhancing the ability of
maintainers, developers and contributors to add features and fix bugs.  More
crucially, code style can also be used to help guard against dangerous code.
While many argue that consistency, rather than a specific style, is the real
goal, I disagree: consistently unreadable and confusing is still unreadable and
confusing.  Therefore, I provide these style guidelines which set out the
format in which D2k source is to be formatted.

## General Guidelines

First, some general guidelines.

### Reformatting

As a derivative of PrBoom+, D2K has accepted contributions from numerous
contributors over its storied lifespan, many of which have different code
styles.  Going forward, contributions should be formatted according to this
style guide, but existing code should be properly reformatted whenever it is
found.  Some projects advise against formatting-only changes; this is not one
of them.

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

Presuming a font that is 7 pixels wide:

  * 1024x768: 146 characters (1, 1)
  * 1152x720: 164 characters (2, 1)
  * 1280x720: 182 characters (2, 1)
  * 1366x768: 195 characters (2, 1)
  * 1440x900: 200 characters (2, 2)
  * 1600x900: 228 characters (2, 2)
  * 1920x1080: 274 characters (3, 2)
  * 2048x1152: 292 characters (3, 2)
  * 2560x1440: 365 characters (4, 3)
  * 2880x1800: 411 characters (5, 4)

100-column buffer widths only match up when using 1440x900 and 1600x900, two
relatively uncommon resolutions.  Everywhere else, particularly the common
resolutions of 1280x720, 1366x768 and 1920x1080, 80-column buffer widths afford
the programmer an extra buffer.  Users of 1024x768 will have to scale down
their font size if they desire more than 1 column regardless of buffer width.

Furthermore, it is becoming more common to view code on mobile devices, and 
longer lines there mean smaller, less readable characters.

It's also worth saying that high-resolution displays are uncommon in the
developing world.  You ought to be able to edit the source code of D2K without
ugly wrapping or painful scrolling no matter where you are, and a minimum
640x480 resolution is entirely reasonable.

Finally, it is much easier to scan through code when it is only 80 characters
wide.

### Spacing

  * Pad operators
    * Bad: `myargv[p+1]`
    * Good: `myargv[p + 1]`
    * Bad: `for (i=0;i<MAXPLAYERS;i++)`
    * Good: `for (i = 0; i < MAXPLAYERS; i++)`
  * Put a space after control statements
    * Bad: `if(`
    * Good: `if (`
  * Do not pad parentheses
    * Bad: `P_SpawnMobj ( x, y, z, MT_PLAYER ) ;`
    * Bad: `if ( debug ) {`
    * Good: `P_SpawnMobj(x, y, z, MT_PLAYER);`
    * Good: `if (debug) {`
  * Pad braces
    * Bad: `if (debug){`
    * Good: `if (debug) {`
  * Pad function parameters
    * Bad: `P_SpawnMobj(x,y,z,MT_PLAYER);`
    * Good: `P_SpawnMobj(x, y, z, MT_PLAYER);`
  * Do not pad function names
    * Bad: `P_SpawnMobj (x, y, z, MT_PLAYER);`
    * Good: `P_SpawnMobj(x, y, z, MT_PLAYER);`
  * Pad commas in general
    * Bad: `int x,y,z,a,b,c;`
    * Good: `int x, y, z, a, b, c;`
  * Do not pad dereferences
    * Bad: `p -> mo = NULL;`
    * Good: `p->mo = NULL;`
  * Pad pointer types in casts, but do not pad the cast itself
    * Bad: `data = (void*) p;`
    * Good: `data = (void *)p;`

## Switch Blocks

Format switch blocks accordingly:

    switch (x) {
      case 0:
      {
        int zero = 0;
        M_DoingSomeThingsHere(zero);
      }
      break;
      case 1:
        M_DoingSomeThingsHere(1);
      break;
      default:
      break;
    }

The `break` is indented at the same level as the `case` because if one creates
a block, ugliness ensues:

    switch (x) {
      case 0:
      {
        int zero = 0;
        M_DoingSomeThingsHere(zero);
      }
        break;
      ...

Putting the `break` inside the block is an option, however that breaks up the
visual consistency of always having the `break` before the `case`, which is
vitally important in checking for fall-through bugs.

## Braces

Braces open on the same line and close on their own line; i.e. OTBS style.
Perhaps an example is best:

    int main(int argc, char **argv) {
      return EXIT_SUCCESS;
    }

This applies to all blocks, functions, structs, enums, standalone blocks,
everything... _except_ switch-case blocks (as in the example above).

### When To Use Braces

Use braces always.

Bad:

    for (int i = 0; i < MAXPLAYERS; i++)
      if (!playeringame[i])
        break;

Bad:

    for (int i = 0; i < MAXPLAYERS; i++) {
      if (!playeringame[i])
        break;
    }

Good:

    for (int i = 0; i < MAXPLAYERS; i++) {
      if (!playeringame[i]) {
        break;
      }
    }

This is to avoid mishaps when diff algorithms.

## Else-If

`else if` goes on a single line.  For example, instead of:

    else
      if

write:

    else if

## Blocks after `return`, `continue`, etc.

There is no need for an `else` or an `else if` after a `return` or `continue`
statement; their existence serves only to confuse, and removing them helps to
save on indentation levels.  For example:

Don't do this:

    if (error) {
      C_Echo("Error!");
      return;
    }
    else {
      C_Echo("No problem!");
    }

Do this:

    if (error) {
      C_Echo("Error!");
      return;
    }

    C_Echo("No problem!");

Keep this in mind as a general rule (it can apply to `I_Error`, etc., as well).

## Line Breaks

Line breaking is a last resort; do not break lines without a reason.  Here are
some examples:

**For Loop**

    for (
      int whoa_this_is_long = 0;
      whoa_this_is_long < whoa_this_is_longer;
      whoa_this_is_long++
    ) {
      /* Blah... */
    }

**If**

    if (this_is_a_long_condition &&
        (this_is_a_longer_condition && this_is_the_longest_condition) &&
        (these_are_very_long_conditions_that_go_on_different_lines_1 &&
         these_are_very_long_conditions_that_go_on_different_lines_2)) {
      /* Blah... */
    }

That said, these are quite ugly, and you should consider a slight refactoring.

Do not put more than one statement on a line, for example, this
violates the style guidelines:

    case pc_unused: fputc(' ', stderr); break;

And do not put single-line control statements on the same line either, i.e.:

    while (i--) printf("BAHAHAHAHAHAHAHAHAHHAA\n");

or:

    if (debug) prinf("This is a debugging printf");

### Function Calls

Function calls should be broken using the following logic:

  * If the arguments fit on a single line by themselves, break after the `(`
    and keep them all on the same line, placing the closing `)` on its own
    line.
  * Otherwise place each argument on its own line.

For example:

    printf(
      "This is a long format string with %d argument", 1
    );

    printf(
      "This is a long format string with multiple arguments: %d, %d, %d.\n",
      1,
      2,
      3
    );

Some functions have what amounts to "boilerplate" for their initial
arguments--`printf` is a good example of this--and therefore you can justify
keeping them on the same line as the function call:

    printf("This is a long format string with many arguments: %d, %d, %d.\n",
      1, 2, 3
    );

    fprintf(stderr, "This is a long error with many arguments: %d, %d, %d.\n",
      1, 2, 3
    );

### "But it still just doesn't fit"

Then it is time to refactor.

## Ternaries

Don't use them.  They're hard to edit when they're small, and they're
difficult to understand when they're large.

## Variable Declarations

### Placement

Put as many variable declarations as you possibly can at the top of the block.
If you have variables that are used only in a nested block, place those
variables in the nested block, not in the top-level block, to improve locality.

Put a line between variable declarations and the body of a block, i.e. instead
of:

    if (blah) {
      unsigned int j = 0;
      printf("Blah: %u.\n", j);
    }

you should write:

    if (blah) {
      unsigned int j = 0;

      printf("Blah: %u.\n", j);
    }

While it seems unnecessary for this small example, it enhances readability
immensely for larger blocks.

### Style

When declaring a pointer type, place the `*` by the variable, not by the type,
for example:

    int main(int argc, char **argv) {
      char *name = NULL;

      return EXIT_SUCCESS;
    }

I sympathize with those who disagree; `char` is not the same type as `char*`,
but this is to avoid inconsistency when declaring a variable list of pointer
types.

Casts can also be difficult to read quickly without a space, therefore employ
a space in these cases as well, i.e.:

    M_BufferWrite(&buf, (void *)important_data, 42);

However, there is no opportunity for inconsistency when declaring a return
value that is a pointer, therefore in that case, keep the `*` with the type:

    const char* M_GetUTFError(int error_code);

## Static Functions

Declare static functions using lowercase and underscores, i.e.:

    static void try_harder(void);

Put all static functions at the top of the file, before non-static functions,
and order them so as to avoid the need for forward declarations.  If this is
impossible, it's likely that you need to refactor.

## Non-Static Functions

Declare non-static functions with their prefix, an underscore, then a capital
camel case name:

    void M_GoodFunctionName(void);

Admittedly, this is not the norm for C, but the majority of the Doom source was
originally written in this style, and when dealing with developers familiar
with Doom, the function names (`P_SpawnMobj`, `P_Move`, etc.) are important
touchstones of communication.  Therefore, editing for style is not entirely
justified.  Besides, the prefix can hold useful information: M is for "misc" or
"menu", N is for "net", R is for "renderer", etc.

## Header files

### Header Includes

Don't `#include` headers in header files.  If you like, you can write a comment
indicating what headers an including C source file should include before it.
If you need something declared in a different header file, forward declare it.
If you need to modify the original header file to do so, feel free to.  This
cuts down on compilation time and avoids complicated dependency issues.

### Header Guards

Always use header guards.  Name them after the file itself, i.e.,
`p_mobj.h` becomes `P_MOBJ_H__`.

The `__` prefix is reserved for compiler defines; do not use it for headers.
For example, instead of:

    #ifndef __P_MOBJ_H__
    #define __P_MOBJ_H__

write:

    #ifndef P_MOBJ_H__
    #define P_MOBJ_H__

### Include Order

  1. `z_zone.h`
  2. Library header files (`<SDL.h>`, `<enet/enet.h>`, etc.)
  3. D2K headers

Include `z_zone.h` first, so that anything afterwards uses its configuration
information and redefinitions.

Include library headers next so that D2K headers can use their definitions.

### Local Headers

Always surround local headers with double-quotation marks, i.e.:

    #include "p_mobj.h"

instead of

    #include <p_mobj.h>

Conversely, always surround non-local headers (like `stddef.h` and
`SDL.h`) with angle brackets, i.e.:

    #include <stddef.h>

    #include <SDL.h>

instead of

    #include "SDL.h"

### System Headers

System headers should all be located in `z_zone.h`.  Practically every file in
the source code includes `z_zone.h`, so if you find yourself in need of a
system header not already located therein, make sure your need is strong.  The
best interface is one that requires no header files besides its own to
function.  Of course, this is not always (or even often) possible, but the
ideal serves well as a goal nonetheless.

## Function Declarations

Functions that take no arguments should be declared as:

    void M_GreatFunction(void);

not:

    void M_GreatFunction();

The latter indicates that the function takes any number of of arguments
(unknown arity), whereas the former indicates that the function takes no
arguments (0 arity).  If you want to leave the arity unspecified use varargs.

## Pre/Post-Increment/Decrement

D2K is written in C, not C++, and the convention in C is to default to
post-increment/decrement.  Do not use pre-increment/decrement unless it is
vital to the algorithm.

Do not tuck increments and decrements in other calls or array accesses.

Bad:

    *result = mf->data[mf->pos++];

Good:

    *result = mf->data[mf->pos];
    mf->pos++;

This is because it is hard to find them, especially when looking for problems.

## Assignment Value

In C, assignment returns a value, i.e.

    if (!(super_struct = malloc(sizeof(super_struct_t))) {
      I_Error("Error allocating super struct\n");
    }

Do not use this.  I sympathize because it's often great shorthand, but in
practice, it creates cluttered and hard to read conditionals.  Rarely does
something like this fit on a single line.

Do not use multiple assignment.  The extra couple lines saved are not worth the
loss of readability or grep-ability.  If you are searching for `id = 0`, you
won't find it if it's tucked in a `id = tag = 0`.

## Checks against NULL/false/0

When checking for `NULL`, `false`, or `0`, just use the bang (`if (!blah)`).  C
compilers handle pointers specially in conditional checks, so there's no need
to worry about conflating `NULL` with `0`, and `false` is defined in the
standard to fail conditional checks as well.  Explicitly checking against the
constant seems like good programming practice, especially because it can give
some implicit type information (pointer vs. boolean vs. integer), but in
general the shorthand is worth it, because the vast majority of these checks
are error cases and you don't care what the tested operand is afterwards
anyway.

<!-- vi: set et ts=2 sw=2 tw=79: -->

