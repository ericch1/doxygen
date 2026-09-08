/** \file three_lines.h
 *  Same as two_lines.h, but with three lines of documentation. Here both
 *  members give the same output, so the problem is no longer visible.
 */

/** Three lines of documentation per member. */
class ThreeLines
{
  public:
    int aligned;      //!< First line of the docs.
                      //!< Second line of the docs.
                      //!< Third line of the docs.

    int notAligned;   //!< First line of the docs.
        //!< Second line of the docs.
        //!< Third line of the docs.
};
