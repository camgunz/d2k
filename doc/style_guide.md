# PrBoom+ Style Guide

Code style is important; it improves readability thus enhancing the ability of
maintainers, developers and contributors to add features and fix bugs.  While
many argue that consistency, rather than a specific style, is the real goal, I
disagree: consistently unreadable and confusing is still unreadable and
confusing.  Therefore, I provide these style guidelines which set out the
format in which PrBoom+ source is to be formatted.

## General Guidelines

First, some general guidelines.

### Reformatting

PrBoom+ has accepted contributions from numerous contributors over its storied
lifespan, many of which have different code styles.  Going forward,
contributions should be formatted according to this style guide, but existing
code should be properly reformatted whenever it is found.  Some projects advise against formatting-only changes; this is not one of them.

### Indenting

PrBoom+ uses an indent-width of 2 spaces.  **Never use tabs for any reason**.

### Tabs vs. Spaces

To repeat for emphasis: **never use tabs for any reason**.  Configure your
editor to convert tabs into 2 spaces.

### Line Width

Adhere strongly to 80-column buffer widths in your editor.  Yes it can be silly
and no we aren't on tiny terminals anymore, but having multiple editing buffers
open horizontally at a time (or browser windows, debuggers, etc.) is a much
bigger increase of productivity than not having to break a line.

A 100-character line width limit would mean that even using a typeface 7 pixels
wide, a monitor with 1920 horizontal pixels could only fully display 2 editor
buffers.  That's not so bad, but the most common resolution on notebooks (as of
this writing) is 1366x768; such notebooks could not even fully display two
editor buffers.  Furthermore, viewing code on mobile devices is more and more
common; on my 1920x1080 Nexus 5 I get roughly 74 characters in landscape mode
when viewing code on GitHub.

It's also worth saying that in the developing world, high-resolution displays
are uncommon.  A 640x480 display ought to be sufficient to edit the source code
of PrBoom+.

Finally, it is much easier to scan through code when it is only 80 characters
wide.

### Spacing

  * Pad operators
    * Bad: `myargv[p+1]`
    * Bad: `for (i=0;i<MAXPLAYERS;i++)`
    * Good: `myargv[p + 1]`
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
  * Pad commas in general
    * Bad: `int x,y,z,a,b,c;`
    * Good: `int x, y, z, a, b, c;`
  * Do not pad dereferences
    * Bad: `p -> mo = NULL`;
    * Good: `p->mo = NULL`;
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

The `break` is indented at the same level as the `case` because ugliness
ensures otherwise if one creates a block:

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

Use braces _almost_ always.  You need not employ braces for single-line
if-statements and loops, But if they contain other control structures, you
**must** use braces.  For example, this is a common loop:

    for (int i = 0; i < MAXPLAYERS; i++)
      if (!playeringame[i])
        break;

This style violates the guidelines because the for loop contains another
control structure.  Rather it should be written either as:

    for (int i = 0; i < MAXPLAYERS; i++) {
      if (!playeringame[i])
        break;
    }

or:

    for (int i = 0; i < MAXPLAYERS; i++) {
      if (!playeringame[i]) {
        break;
      }
    }

The preferred form is the 2nd form.

Furthermore, if any of your if/else if/else blocks are not single-line blocks,
use braces for all of them.  For example, instead of:

    if (x == 0)
      return false;
    else if (x == 1) {
      printf("X: 1\n");
      return true;
    }
    else
      I_Error("I_BadStyle: Got invalid value for x: %d.", x);

write:

    if (x == 0) {
      return false;
    }
    else if (x == 1) {
      printf("X: 1\n");
      return true;
    }
    else {
      I_Error("I_BadStyle: Got invalid value for x: %d.", x);
    }

## Multi-line Control Statements

If your control statement spans more than one line, you must use braces.

## Else-If

`else if` goes on a single line.  For example, instead of:

    else
      if

write:

    else if

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

That said, these are very ugly, and you should consider a slight refactoring.

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
arguments--printf is a good example of this--and therefore you can justify
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
impossible to understand when they're large.

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

I sympathize with those who disagree, `char` is not the same type as `char*`,
but this is to avoid inconsistency when declaring a variable list of pointer
types.

However, there is no opportunity for inconsistency when declaring a return
value that is a pointer, therefore in that case, keep the `*` with the type:

    const char* M_GetUTFError(int error_code);

## Static Functions

Declare static functions using lowercase and underscores, i.e.:

    static void try_harder(void);

## Non-Static Functions

Declare non-static functions with their prefix, an underscore, then a capital
camel case name:

    void M_GoodFunctionName(void);

## Includes

### Header Includes

Don't `#include` headers in header files.  If you like, you can write a comment
indicating what headers an including C source file should include before it.
If you need something declared in a different header file, forward declare it.
If you need to modify the original header file to do so, feel free to.  This
cuts down on compilation time and avoids complicated dependency problems.

### Header Defines

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
  3. PrBoom+ headers

Include `z_zone.h` first, so that anything afterwards uses its configuration
information and redefinitions.

Include library headers next so that PrBoom+ headers can use their definitions.

## Function Declarations

Functions that take no arguments should be declared as:

    void M_GreatFunction(void);

not:

    void M_GreatFunction();

