// objective: test that a member comment on several lines gives the same result whether the markers start in the same column or not (see issue #5266)
// check: class_after_member.xml
// check: class_before_member.xml
// check: class_still_separate.xml
// check: class_explicit_brief.xml
// config: EXTRACT_ALL = YES

/** \file
 *  A comment block that follows a description of the same member is part of
 *  that description, so the column its markers start in must not change the
 *  result. This holds both for a block written after a member and for a block
 *  written in front of one.
 */

/** Blocks written after the member. */
class AfterMember
{
  public:
    /* two lines, markers in the same column */
    int twoAligned;      //!< Line 1.
                         //!< Line 2.

    /* two lines, second marker in another column */
    int twoMisaligned;   //!< Line 1.
        //!< Line 2.

    /* three lines, only the last marker in another column */
    int threeLastOut;    //!< Line 1.
                         //!< Line 2.
        //!< Line 3.

    /* the same, written with ///< */
    int jdAligned;       ///< Line 1.
                         ///< Line 2.

    int jdMisaligned;    ///< Line 1.
        ///< Line 2.
};

/** Blocks written in front of the member. */
class BeforeMember
{
  public:
    //! Line 1.
    //! Line 2.
    int aligned;

    //! Line 1.
        //! Line 2.
    int misaligned;
};

/** A block that belongs to the next member must stay separate. */
class StillSeparate
{
  public:
    int a; //!< doc for a
//! doc for b
    int b;
};

/** An explicit \\brief keeps the brief whatever comes before it. */
class ExplicitBrief
{
  public:
    /*! \details Details first.
     *  \brief The real brief.
     */
    int x;

    //! Brief description.

    //! Detailed description
    //! starts here.
    int y;
};
