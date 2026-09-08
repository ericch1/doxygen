/** \file matrix.h
 *  All the marker layouts we tried. The column of the first `//` marker is
 *  the reference; the shift given for each case is relative to it.
 */

/** Sequences of two comment lines. */
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

    // H: three lines, only the third one out of line
    int h;           //!< Line 1.
                     //!< Line 2.
        //!< Line 3.

    // I: three lines, the last two out of line but in the same column
    int i;           //!< Line 1.
        //!< Line 2.
        //!< Line 3.
};

/** Longer sequences. */
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
