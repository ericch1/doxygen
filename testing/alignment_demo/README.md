# `//!<` marker alignment and the automatic `\brief`

A small test case showing that a member comment written on several lines
**does not give the same output depending on whether the `//!<` (or `///<`)
markers are in the same column or not**.

Tested with doxygen **1.19.0** (`master`, commit `ae4c402`) using the default
settings (`MULTILINE_CPP_IS_BRIEF = NO`, `JAVADOC_AUTOBRIEF = NO`,
`QT_AUTOBRIEF = NO`).

## How to run

```sh
./run.sh /path/to/doxygen
```

## The rule behind it

The column is not really what matters. What matters is **what happens to the
last line of the sequence**:

> `commentcnv` joins into one `/*!< ... */` block every run of `//!<` lines
> that are in the same column. The **last** line of the whole sequence is
> never rewritten and stays a one-line `//!<`, which doxygen reads as a
> **brief** description — unless the column test pulled it into the block
> just before it.

So the problem only shows up when the last line is **alone in its column**.

## Two lines: the problem shows up (`two_lines.h`)

```cpp
    int aligned;      //!< First line of the docs.
                      //!< Second line of the docs.

    int notAligned;   //!< First line of the docs.
        //!< Second line of the docs.
```

| member       | `briefdescription`         | `detaileddescription`                     |
|--------------|----------------------------|--------------------------------------------|
| `aligned`    | *(empty)*                  | `First line of the docs. Second line of the docs.` |
| `notAligned` | `Second line of the docs.` | `First line of the docs.`                  |

Two things you can see in the HTML output. In the "Public Attributes" table,
`aligned` has **no** short description while `notAligned` has one. And in
"Member Data Documentation" the two lines of `notAligned` come out **in the
wrong order**, because the brief is printed before the detailed part:

```
* aligned
  First line of the docs. Second line of the docs.
* notAligned
  Second line of the docs.
  First line of the docs.
```

## Three lines: the problem goes away (`three_lines.h`)

Same code with one more line of documentation for each member:

| member       | `briefdescription` | `detaileddescription`                          |
|--------------|--------------------|-------------------------------------------------|
| `aligned`    | *(empty)*          | `First line. Second line. Third line.`           |
| `notAligned` | *(empty)*          | `First line. Second line. Third line.`           |

Both are now the same. Here is how doxygen rewrites the comments
(`doxygen -d commentcnv`):

```cpp
    int aligned;      /*!< First line of the docs.
                        *  Second line of the docs.
                        *  Third line of the docs. */

    int notAligned;   /*!< First line of the docs. */
        /*!< Second line of the docs.
          *  Third line of the docs. */
```

The second member still gets **two** blocks instead of one, but lines 2 and 3
are in the same column, so line 3 joins the second block. Two detailed blocks,
put one after the other, in the right order. No brief is created, and the
output happens to be correct.

## All the cases we tried (`matrix.h`)

Two-line sequences (class `Matrix`). The shift is measured from the column of
the first `//` marker:

| case | marker  | shift of line 2 | brief     | detailed          |
|------|---------|-----------------|-----------|--------------------|
| `a`  | `//!<`  | 0               | *(empty)* | `Line 1. Line 2.` |
| `b`  | `//!<`  | -13 (left)      | `Line 2.` | `Line 1.`         |
| `c`  | `//!<`  | +3 (right)      | `Line 2.` | `Line 1.`         |
| `d`  | `//!<`  | -1 (left)       | `Line 2.` | `Line 1.`         |
| `e`  | `//!<`  | **+1 (right)**  | *(empty)* | `Line 1. Line 2.` |
| `f`  | `///<`  | 0               | *(empty)* | `Line 1. Line 2.` |
| `g`  | `///<`  | -13 (left)      | `Line 2.` | `Line 1.`         |

Longer sequences (class `Matrix3`):

| case | layout                                                | brief     | detailed                     |
|------|-------------------------------------------------------|-----------|-------------------------------|
| `j`  | 3 lines, all in the same column                       | *(empty)* | `Line 1. Line 2. Line 3.`     |
| `k`  | 3 lines, last two out of line but in the same column  | *(empty)* | `Line 1. Line 2. Line 3.`     |
| `l`  | 3 lines, only the **third** one out of line           | `Line 3.` | `Line 1. Line 2.`             |
| `m`  | 3 lines, only the second out of line, third one back in the first column | `Line 3.` | `Line 1. Line 2.` |
| `n`  | 3 lines, three different columns                      | `Line 3.` | `Line 1. Line 2.`             |
| `o`  | 4 lines, last three out of line but in the same column| *(empty)* | `Line 1. ... Line 4.`         |

Case `m` is worth a look. Line 3 is back in the same column as line 1, but it
does not join the first block, because that block was already closed by
line 2. So line 3 ends up alone and becomes the brief:

```cpp
    int m;           /*!< Line 1. */
        /*!< Line 2. */
                     //!< Line 3.
```

## Where this happens in the code

Two parts of `src/commentcnv.l` work together here.

1. The opening rule only turns a `//!` into a `/*!` when the **next** line
   also starts with `//!` or `///`:

   ```
   <Scan>{CPPC}"!"/.*\n[ \t]*{CPPC}[\/!][^\/]
   ```

   So the last line of a sequence is never rewritten. It stays a one-line
   `//!<`, which means a brief.

2. `replaceComment()` decides whether a line goes on with the current block:

   ```cpp
   size_t i=computeIndent(&yytext[offset]);
   if (i==yyextra->blockHeadCol || i+1==yyextra->blockHeadCol)
   {
     replaceCommentMarker(...);      // keep the current block
   }
   else
   {
     copyToOutput(yyscanner," */");  // close the block, go back to <Scan>
     ...
   }
   ```

   `blockHeadCol` is `col+1`, where `col` is the column of the opening `//`.
   So a line goes on with the block only when its indent is **exactly the
   column of the opening `//`, or that column plus one**. That is why case `d`
   (one column to the left) breaks the block while case `e` (one column to the
   right) does not.

## What the documentation says

* The user manual (`doc/docblocks.dox`, section "Putting documentation after
  members") shows a one-line `//!<` for a brief, and a `//!<` followed by an
  empty `//!<` line for a detailed description. Both examples have the markers
  in the same column, but the text **never says that the column matters**.
* `MULTILINE_CPP_IS_BRIEF` (`src/config.xml`) talks about "a block of `//!` or
  `///` comments" without saying what makes a block.
* The only place that describes the rule is the **1.3.1 changelog**
  (`doc/changelog.dox`), under "Changes":

  > A multi-line C++ comment block now has to be aligned in order to make
  > doxygen treat it is one block. [...] Hopefully this will give more
  > intuitive results. Tabs are replaced by spaces according to the value of
  > TAB_SIZE in the config file.

  The entry then gives two examples, one said to be a single block and one
  said to be two blocks. **Both examples have the markers in the same
  column**, so they do not show the difference; the only change between them
  is one space *after* the `//!`. Running them through doxygen 1.19.0 gives
  the same result for both.

So the behaviour was on purpose, but it was written down once, in 2003, in a
changelog, with a broken example, and it never made it into the user manual.

## Git history

* `git log -S blockHeadCol -- src/commentcnv.l` points at
  **`20bd371f`, 2003-05-14** (Release-1.3-20030514), the release that matches
  the changelog entry above. Before it, `replaceCommentMarker()` was called
  for every continuation line, so the column was not looked at at all. That
  commit added `g_blockHeadCol`, `computeIndent()` and the test
  `if (i==g_blockHeadCol)`.
* The `|| i+1==blockHeadCol` part came much later, in **`38ee114a`,
  2024-06-23**, a commit titled *"issue #10935 \snippet{doc} tag in Doxygen
  v1.11 adds incorrect paragraph with a break before snippet text"*. That
  commit reworked column tracking and changed `blockHeadCol` from `col` to
  `col+1` in several places. The one-column tolerance on the right looks like
  a side effect of that change, not a decision about alignment.
* `c3ce689d` (2026-08-17) only changed `int` to `size_t` and replaced a
  `yyless()` by an `unput()` loop.

## Related report

[doxygen/doxygen#5266](https://github.com/doxygen/doxygen/issues/5266),
*"Multi-line \"after the member\" ///< comment works incorrectly"*
(from bugzilla #706084, 2013), is the same problem and is still **open**. The
reporter shows `///<` continuation lines that are not in the column of the
first marker, says an empty comment line works around it, and later notes he
gets "reversed descriptions" — the order swap shown above.

## Which languages

This is not a C++ only thing: `commentcnv` handles every language with `//`
comments the same way. We checked and got the same result in **C**, **C++**
and **Java**. Python (`##`) goes through another path and is not affected.
