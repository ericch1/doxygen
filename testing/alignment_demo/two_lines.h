/** \file two_lines.h
 *  Two members, each documented on two lines with the `//!<` continuation
 *  marker. The first member has both markers in the same column, the
 *  second one does not. The output is not the same.
 */

/** Two lines of documentation per member. */
class TwoLines
{
  public:
    int aligned;      //!< First line of the docs.
                      //!< Second line of the docs.

    int notAligned;   //!< First line of the docs.
        //!< Second line of the docs.
};
