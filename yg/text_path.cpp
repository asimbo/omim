#include "../base/SRC_FIRST.hpp"

#include "text_path.hpp"

#include "../geometry/angles.hpp"

namespace yg
{
  PathPoint::PathPoint(int i, m2::PointD const & pt)
    : m_i(i),
      m_pt(pt)
  {}

  PivotPoint::PivotPoint(double angle, PathPoint const & pp)
    : m_angle(angle), m_pp(pp)
  {}
  TextPath::TextPath(m2::PointD const * arr, size_t sz, double fullLength, double & pathOffset)
    : m_arr(arr), m_size(sz), m_reverse(false)
  {
    ASSERT ( m_size > 1, () );

    /* assume, that readable text in path should be ('o' - start draw point):
     *    /   o
     *   /     \
     *  /   or  \
     * o         \
     */

    double const a = ang::AngleTo(m_arr[0], m_arr[m_size-1]);
    if (fabs(a) > math::pi / 2.0)
    {
      // if we swap direction, we need to recalculate path offset from the end
      double len = 0.0;
      for (size_t i = 1; i < m_size; ++i)
        len += m_arr[i-1].Length(m_arr[i]);

      pathOffset = fullLength - pathOffset - len;
      ASSERT ( pathOffset >= -1.0E-6, () );
      if (pathOffset < 0.0) pathOffset = 0.0;

      m_reverse = true;
    }
  }

  size_t TextPath::size() const { return m_size; }

  m2::PointD TextPath::get(size_t i) const
  {
    ASSERT ( i < m_size, ("Index out of range") );
    return m_arr[m_reverse ? m_size - i - 1 : i];
  }

  m2::PointD TextPath::operator[](size_t i) const { return get(i); }

  PathPoint const TextPath::offsetPoint(PathPoint const & pp, double offset)
  {
    PathPoint res = pp;

    if (res.m_i == -1)
      return res;

    for (size_t i = res.m_i; i < size() - 1; ++i)
    {
      double l = res.m_pt.Length(get(i + 1));
      res.m_pt = res.m_pt.Move(min(l, offset), ang::AngleTo(get(i), get(i + 1)));
      res.m_i = i;

      if (offset <= l)
        break;
      else
        offset -= l;
    }

    return res;

  }

  PivotPoint TextPath::findPivotPoint(PathPoint const & pp, GlyphMetrics const & sym, double kern)
  {
    PathPoint startPt = offsetPoint(pp, kern);

    PivotPoint res;
    res.m_pp.m_i = -1;
    m2::PointD pt1 = startPt.m_pt;

    double angle = 0;
    double advance = sym.m_xOffset + sym.m_width / 2.0;

    int j = startPt.m_i;

    while (advance > 0)
    {
      if (j + 1 == size())
        return res;

      double l = get(j + 1).Length(pt1);

      angle += ang::AngleTo(get(j), get(j + 1));

      if (l < advance)
      {
        advance -= l;
        pt1 = get(j + 1);
        ++j;
      }
      else
      {
        res.m_pp.m_i = j;
        res.m_pp.m_pt = pt1.Move(advance, ang::AngleTo(get(j), get(j + 1)));
        advance = 0;

        angle /= (res.m_pp.m_i - startPt.m_i + 1);
        res.m_angle = angle;

        break;
      }
    }

    return res;
  }
}

