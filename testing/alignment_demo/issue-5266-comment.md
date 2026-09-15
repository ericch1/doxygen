<!-- Draft of a comment to post on https://github.com/doxygen/doxygen/issues/5266 -->

This issue is labelled `needinfo`, so here is what we found. It also answers the
question left open in the last comment (2015-06-22), where the descriptions came
out in a "reversed" order and nobody could say why.

Everything below was run with **doxygen 1.19.0, `master` at `ae4c402`**, built
from source, with **default settings** (`MULTILINE_CPP_IS_BRIEF = NO`,
`JAVADOC_AUTOBRIEF = NO`, `QT_AUTOBRIEF = NO`).

## Minimal example

`bug.h`:

```cpp
class Demo
{
  public:
    int aligned;      //!< First line of the docs.
                      //!< Second line of the docs.

    int notAligned;   //!< First line of the docs.
        //!< Second line of the docs.
};
```

`Doxyfile` (everything not listed keeps its default value):

```
PROJECT_NAME           = "bug"
INPUT                  = bug.h
OUTPUT_DIRECTORY       = out
EXTRACT_ALL            = YES
GENERATE_HTML          = YES
GENERATE_XML           = YES
GENERATE_LATEX         = NO
QUIET                  = YES
```

Run with `doxygen Doxyfile`.

## What comes out

From `out/xml/classDemo.xml`:

| member       | `briefdescription`         | `detaileddescription`                              |
|--------------|----------------------------|-----------------------------------------------------|
| `aligned`    | *(empty)*                  | `First line of the docs. Second line of the docs.`  |
| `notAligned` | `Second line of the docs.` | `First line of the docs.`                           |

In the HTML output this shows up twice. The "Public Attributes" table gives
`aligned` no short description at all, while `notAligned` gets one:

```
Public Attributes
  int  aligned
  int  notAligned
       Second line of the docs.
```

And in "Member Data Documentation" the two lines of `notAligned` come out in the
opposite order from the source, because the brief is printed before the detailed
description:

```
* aligned
  First line of the docs. Second line of the docs.
* notAligned
  Second line of the docs.
  First line of the docs.
```

That second block is the "reversed" order reported in 2015.

## Why

`doxygen -d commentcnv` shows how `src/commentcnv.l` rewrites the C++ comments
before anything else parses them:

```cpp
    int aligned;      /*!< First line of the docs.
                        *  Second line of the docs. */

    int notAligned;   /*!< First line of the docs. */
        //!< Second line of the docs.
```

For `aligned` the two lines become **one** `/*!< ... */` block. A `/*! */` block
is a detailed description by default, so there is no brief.

For `notAligned` the marker in another column closes the block and a second one
starts. The member now has two documentation blocks: a `/*!< ... */` (detailed)
and a one line `//!<`. A one line `//!<` **is** a brief description, so the
second line of the comment becomes the `\brief` of the member.

Two parts of `src/commentcnv.l` produce this together.

1. The opening rule only turns a `//!` into a `/*!` when the **next** line also
   starts with `//!` or `///`:

   ```
   <Scan>{CPPC}"!"/.*\n[ \t]*{CPPC}[\/!][^\/]
   ```

   So the last line of a sequence is never rewritten. It stays a one line
   `//!<`, which means a brief.

2. `replaceComment()` decides whether a line continues the current block:

   ```cpp
   size_t i=computeIndent(&yytext[offset]);
   if (i==yyextra->blockHeadCol || i+1==yyextra->blockHeadCol)
   {
     replaceCommentMarker(...);      // keep the current block
   }
   else
   {
     copyToOutput(yyscanner," */");  // close the block, back to <Scan>
     ...
   }
   ```

So the rule is:

> Every run of `//!<` lines in the same column is joined into one
> `/*!< ... */` block. The **last** line of the whole sequence is never
> rewritten and stays a one line `//!<`, i.e. a brief, unless the column test
> pulled it into the block before it.

The problem therefore only shows up when the last line is **alone in its
column**.

## Why this looked inconsistent

Adding a third line to the same misaligned comment makes the difference go away:

```cpp
    int notAligned;   //!< First line of the docs.
        //!< Second line of the docs.
        //!< Third line of the docs.
```
gives the same output as the aligned version: no brief, everything detailed.
Lines 2 and 3 are in the same column, so line 3 joins the second block, and two
detailed blocks are simply concatenated in order.

This is probably why the original report was hard to confirm: whether the bug
appears depends on how many lines the comment has and which of them are out of
line, not just on "the markers are not aligned".

<details>
<summary>All the layouts we measured (click to expand, with the source)</summary>

```cpp
class Matrix
{
  public:
    // A: both markers in the same column
    int a;           //!< Line 1.
                     //!< Line 2.

    // B: second marker far to the left
    int b;           //!< Line 1.
        //!< Line 2.

    // C: second marker 3 columns to the right
    int c;           //!< Line 1.
                        //!< Line 2.

    // D: second marker 1 column to the left
    int d;           //!< Line 1.
                    //!< Line 2.

    // E: second marker 1 column to the right
    int e;           //!< Line 1.
                      //!< Line 2.

    // F: same as A, but with ///<
    int f;           ///< Line 1.
                     ///< Line 2.

    // G: same as B, but with ///<
    int g;           ///< Line 1.
        ///< Line 2.
};

class Matrix3
{
  public:
    // J: three lines, all in the same column
    int j;           //!< Line 1.
                     //!< Line 2.
                     //!< Line 3.

    // K: three lines, the last two out of line but in the same column
    int k;           //!< Line 1.
        //!< Line 2.
        //!< Line 3.

    // L: three lines, only the third one out of line
    int l;           //!< Line 1.
                     //!< Line 2.
        //!< Line 3.

    // M: three lines, only the second one out of line, the third one back
    //    in the column of the first
    int m;           //!< Line 1.
        //!< Line 2.
                     //!< Line 3.

    // N: three lines, three different columns
    int n;           //!< Line 1.
       //!< Line 2.
             //!< Line 3.

    // O: four lines, the last three out of line but in the same column
    int o;           //!< Line 1.
        //!< Line 2.
        //!< Line 3.
        //!< Line 4.
};
```

| case | layout | brief | detailed |
|------|--------|-----------|-----------|
| `a` | 2 lines, same column | *(empty)* | `Line 1. Line 2.` |
| `b` | 2 lines, 13 columns to the left | `Line 2.` | `Line 1.` |
| `c` | 2 lines, 3 columns to the right | `Line 2.` | `Line 1.` |
| `d` | 2 lines, 1 column to the left | `Line 2.` | `Line 1.` |
| `e` | 2 lines, **1 column to the right** | *(empty)* | `Line 1. Line 2.` |
| `f` | same as `a` with `///<` | *(empty)* | `Line 1. Line 2.` |
| `g` | same as `b` with `///<` | `Line 2.` | `Line 1.` |
| `j` | 3 lines, same column | *(empty)* | `Line 1. Line 2. Line 3.` |
| `k` | 3 lines, last two together out of line | *(empty)* | `Line 1. Line 2. Line 3.` |
| `l` | 3 lines, only the third out of line | `Line 3.` | `Line 1. Line 2.` |
| `m` | 3 lines, only the second out of line | `Line 3.` | `Line 1. Line 2.` |
| `n` | 3 lines, three different columns | `Line 3.` | `Line 1. Line 2.` |
| `o` | 4 lines, last three together out of line | *(empty)* | `Line 1. ... Line 4.` |

Two of these are worth a look.

`e` versus `d`: one column to the **right** keeps the block, one column to the
**left** breaks it. This comes straight from the `i+1==blockHeadCol` half of the
test above.

`m`: line 3 is back in the same column as line 1, but it does not rejoin the
first block, because that block was already closed by line 2. It ends up alone
and becomes the brief:

```cpp
    int m;           /*!< Line 1. */
        /*!< Line 2. */
                     //!< Line 3.
```

</details>

## Which languages

`commentcnv` treats every language with `//` comments the same way. We got the
same result in **C**, **C++** and **Java**. **Python** (`##`), **Fortran**
(`!<`) and **VHDL** (`--!`) go through other paths and show no difference
between the two layouts.

## What the documentation says

* The user manual (`doc/docblocks.dox`, "Putting documentation after members")
  shows a one line `//!<` for a brief, and a `//!<` followed by an empty `//!<`
  line for a detailed description. Both examples have the markers in the same
  column, but the text never says that the column matters. There is no mention
  of alignment, columns or indentation anywhere in that file.
* `MULTILINE_CPP_IS_BRIEF` speaks of "a block of `//!` or `///` comments"
  without saying what makes a block.
* The only place the rule is written down is the **1.3.1 changelog**
  (`doc/changelog.dox`), under "Changes":

  > A multi-line C++ comment block now has to be aligned in order to make
  > doxygen treat it is one block. [...] Hopefully this will give more intuitive
  > results. Tabs are replaced by spaces according to the value of TAB_SIZE in
  > the config file.

  The entry then gives one example said to be a single block and one said to be
  two blocks. **Both examples have the markers in the same column** (column 20),
  so they do not show the difference; the only change between them is one space
  *after* the `//!`. Running both through 1.19.0 gives the same result for both.

So the behaviour was on purpose, but it was written down once, in 2003, in a
changelog, with an example that does not demonstrate it, and it never reached
the user manual.

## History

* `git log -S blockHeadCol -- src/commentcnv.l` points at **`20bd371f`
  (2003-05-14, Release-1.3-20030514)**, the release matching the changelog entry
  above. Before it, `replaceCommentMarker()` was called for every continuation
  line and the column was not looked at at all. That commit added
  `g_blockHeadCol`, `computeIndent()` and the test `if (i==g_blockHeadCol)`.
* The `|| i+1==blockHeadCol` half came much later, in **`38ee114a`
  (2024-06-23)**, titled *"issue #10935 \snippet{doc} tag in Doxygen v1.11 adds
  incorrect paragraph with a break before snippet text"*. That commit reworked
  column tracking and changed `blockHeadCol` from `col` to `col+1` in several
  places. The one column tolerance on the right looks like a side effect of that
  work rather than a decision about alignment.

This is worth noting on its own: `blockHeadCol` is used for three unrelated
jobs - the alignment test, re-indenting content inserted by `\include` and
`\snippet` (`insertCommentStart()`) and multi-line alias expansion
(`replaceAliases()`), and as a "am I inside a C comment" flag. There is no test
in `testing/` covering marker alignment, so a change made for one of the other
jobs can move this behaviour without anyone noticing. That already happened once.

## Question

Before proposing a patch we would like to know which output you want for the
misaligned case:

1. **Same as the aligned case** - the continuation line joins the block, so no
   brief is created and the lines stay in source order. A marker ending in `<`
   can only document the member in front of it, so a following comment line can
   only be about that same member; the column carries no information there. The
   2003 rule would stay in force for `//!` and `///` comments in front of a
   member, where it does carry information.
2. **A brief plus a detailed part** - the first line becomes the brief and the
   rest the detailed description. Note this only works when the first block is a
   single line; in case `l` above the first block has two lines, so it cannot
   provide a brief and the last line would still win.
3. **No behaviour change, but a warning** when a block is split because a marker
   is not in the same column as the previous one.

We have a small patch for option 1, tested against the full `ctest` suite
(122/123, the one failure is `012_cite` which needs `bibtex` and fails without
the patch too) and against the layouts above. Happy to open a PR once you say
which behaviour you prefer.

We can also send a documentation-only PR describing the rule in
`doc/docblocks.dox`, independently of any code change.

---
_Generated by [Claude Code](https://claude.ai/code)_
