/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#ifdef __GNUC__
// under gcc, inline will only take place if optimizations are applied (-O). this will force inline even whith optimizations.
#define XBMC_FORCE_INLINE __attribute__((always_inline))
#else
#define XBMC_FORCE_INLINE
#endif

#include <vector>
#include <algorithm>
#include <cmath>

template <typename T> class CPointGen
{
public:
  typedef CPointGen<T> this_type;

  CPointGen<T>()
  {
    x = 0; y = 0;
  };

  CPointGen<T>(T a, T b)
  {
    x = a;
    y = b;
  };

  template <class U> CPointGen<T>(const CPointGen<U>& rhs)
  {
    x = rhs.x;
    y = rhs.y;
  }

  this_type operator+(const this_type &point) const
  {
    this_type ans;
    ans.x = x + point.x;
    ans.y = y + point.y;
    return ans;
  };

  const this_type &operator+=(const this_type &point)
  {
    x += point.x;
    y += point.y;
    return *this;
  };

  this_type operator-(const this_type &point) const
  {
    CPointGen<T> ans;
    ans.x = x - point.x;
    ans.y = y - point.y;
    return ans;
  };

  const this_type &operator-=(const this_type &point)
  {
    x -= point.x;
    y -= point.y;
    return *this;
  };

  T x, y;
};

template <typename T> class CShapeGen
{
public:
  virtual ~CShapeGen() { }
  virtual CShapeGen<T>* Clone() const = 0;
  virtual bool ContainsPt(const CPointGen<T> &point) const = 0;
  virtual bool IsEmpty() const = 0;
  virtual CPointGen<T> Center() const = 0;
  virtual T Area() const = 0;
  virtual T MinRadius() const = 0;
  virtual T MaxRadius() const = 0;
};

template <typename T> class CCircleGen : public CShapeGen<T>
{
public:
  typedef CCircleGen<T> this_type;

  CCircleGen<T>(void) : x(0), y(0), r(0) { }
  CCircleGen<T>(T x, T y, T r) : x(x), y(y), r(r) { }
  CCircleGen<T>(const CPointGen<T> &center, float radius)
  {
    x = center.x;
    y = center.y;
    r = radius;
  }

  template <class U> CCircleGen<T>(const CCircleGen<U>& rhs)
  {
    x = rhs.x;
    y = rhs.y;
    r = rhs.r;
  }

  virtual CShapeGen<T>* Clone() const
  {
    return new this_type(*this);
  }

  void SetCircle(T x, T y, T r) { this->x = x; this->y = y; this->r = r; }

  bool operator ==(const this_type &circle) const
  {
    return x == circle.x &&
           y == circle.y &&
           r == circle.r;
  }

  bool operator !=(const this_type &circle) const { return !operator==(circle); }

  bool ContainsPt(const CPointGen<T> &point) const
  {
    return std::sqrt((x - point.x) * (x - point.x) + (y - point.y) * (y - point.y)) <= r;
  }

  inline bool IsEmpty() const XBMC_FORCE_INLINE
  {
    return r == 0;
  }

  inline CPointGen<T> Center() const XBMC_FORCE_INLINE
  {
    return CPointGen<T>(x, y);
  }

  inline T Radius() const XBMC_FORCE_INLINE
  {
    return r;
  }

  inline T MinRadius() const XBMC_FORCE_INLINE
  {
    return r;
  }

  inline T MaxRadius() const XBMC_FORCE_INLINE
  {
    return r;
  }

  inline T Area() const XBMC_FORCE_INLINE
  {
    return M_PI * r * r;
  }

  T x, y, r;
};

template <typename T> class CRectGen : public CShapeGen<T>
{
public:
  typedef CRectGen<T> this_type;

  CRectGen<T>() { x1 = y1 = x2 = y2 = 0;};
  CRectGen<T>(T left, T top, T right, T bottom) { x1 = left; y1 = top; x2 = right; y2 = bottom; };
  CRectGen<T>(const CPointGen<T> &p1, const CPointGen<T> &p2)
  {
    x1 = p1.x;
    y1 = p1.y;
    x2 = p2.x;
    y2 = p2.y;
  }

  template <class U> CRectGen<T>(const CRectGen<U>& rhs)
  {
    x1 = rhs.x1;
    y1 = rhs.y1;
    x2 = rhs.x2;
    y2 = rhs.y2;
  }

  virtual CShapeGen<T>* Clone() const
  {
    return new this_type(*this);
  }

  void SetRect(T left, T top, T right, T bottom) { x1 = left; y1 = top; x2 = right; y2 = bottom; };

  bool ContainsPt(const CPointGen<T> &point) const
  {
    if (x1 <= point.x && point.x <= x2 && y1 <= point.y && point.y <= y2)
      return true;
    return false;
  };

  inline const this_type &operator -=(const CPointGen<T> &point) XBMC_FORCE_INLINE
  {
    x1 -= point.x;
    y1 -= point.y;
    x2 -= point.x;
    y2 -= point.y;
    return *this;
  };

  inline const this_type &operator +=(const CPointGen<T> &point) XBMC_FORCE_INLINE
  {
    x1 += point.x;
    y1 += point.y;
    x2 += point.x;
    y2 += point.y;
    return *this;
  };

  const this_type &Intersect(const this_type &rect)
  {
    x1 = clamp_range(x1, rect.x1, rect.x2);
    x2 = clamp_range(x2, rect.x1, rect.x2);
    y1 = clamp_range(y1, rect.y1, rect.y2);
    y2 = clamp_range(y2, rect.y1, rect.y2);
    return *this;
  };

  const this_type &Union(const this_type &rect)
  {
    if (IsEmpty())
      *this = rect;
    else if (!rect.IsEmpty())
    {
      x1 = std::min(x1,rect.x1);
      y1 = std::min(y1,rect.y1);

      x2 = std::max(x2,rect.x2);
      y2 = std::max(y2,rect.y2);
    }

    return *this;
  };

  inline bool IsEmpty() const XBMC_FORCE_INLINE
  {
    return (x2 - x1) * (y2 - y1) == 0;
  };

  inline CPointGen<T> P1() const XBMC_FORCE_INLINE
  {
    return CPointGen<T>(x1, y1);
  }

  inline CPointGen<T> P2() const XBMC_FORCE_INLINE
  {
    return CPointGen<T>(x2, y2);
  }

  inline T Width() const XBMC_FORCE_INLINE
  {
    return x2 - x1;
  };

  inline T Height() const XBMC_FORCE_INLINE
  {
    return y2 - y1;
  };

  inline T Area() const XBMC_FORCE_INLINE
  {
    return Width() * Height();
  };

  inline CPointGen<T> Center() const XBMC_FORCE_INLINE
  {
    return CPointGen<T>((x1 + x2) / 2, (y1 + y2) / 2);
  }

  inline T MinRadius() const XBMC_FORCE_INLINE
  {
    return std::min(Width(), Height()) / 2;
  }

  inline T MaxRadius() const XBMC_FORCE_INLINE
  {
    return std::sqrt(Width() * Width() + Height() * Height()) / 2;
  }

  inline CCircleGen<T> InscribedCircle() const XBMC_FORCE_INLINE
  {
    return CCircleGen<T>(Center(), MinRadius());
  }

  inline CCircleGen<T> Circumcircle() const XBMC_FORCE_INLINE
  {
    return CCircleGen<T>(Center(), MaxRadius());
  }

  std::vector<this_type> SubtractRect(this_type splitterRect)
  {
    std::vector<this_type> newRectaglesList;
    this_type intersection = splitterRect.Intersect(*this);

    if (!intersection.IsEmpty())
    {
      this_type add;

      // add rect above intersection if not empty
      add = this_type(x1, y1, x2, intersection.y1);
      if (!add.IsEmpty())
        newRectaglesList.push_back(add);

      // add rect below intersection if not empty
      add = this_type(x1, intersection.y2, x2, y2);
      if (!add.IsEmpty())
        newRectaglesList.push_back(add);

      // add rect left intersection if not empty
      add = this_type(x1, intersection.y1, intersection.x1, intersection.y2);
      if (!add.IsEmpty())
        newRectaglesList.push_back(add);

      // add rect right intersection if not empty
      add = this_type(intersection.x2, intersection.y1, x2, intersection.y2);
      if (!add.IsEmpty())
        newRectaglesList.push_back(add);
    }
    else
    {
      newRectaglesList.push_back(*this);
    }

    return newRectaglesList;
  }

  std::vector<this_type> SubtractRects(std::vector<this_type > intersectionList)
  {
    std::vector<this_type> fragmentsList;
    fragmentsList.push_back(*this);

    for (typename std::vector<this_type>::iterator splitter = intersectionList.begin(); splitter != intersectionList.end(); ++splitter)
    {
      typename std::vector<this_type> toAddList;

      for (typename std::vector<this_type>::iterator fragment = fragmentsList.begin(); fragment != fragmentsList.end(); ++fragment)
      {
        std::vector<this_type> newFragmentsList = fragment->SubtractRect(*splitter);
        toAddList.insert(toAddList.end(), newFragmentsList.begin(), newFragmentsList.end());
      }

      fragmentsList.clear();
      fragmentsList.insert(fragmentsList.end(), toAddList.begin(), toAddList.end());
    }

    return fragmentsList;
  }

  bool operator !=(const this_type &rect) const
  {
    if (x1 != rect.x1) return true;
    if (x2 != rect.x2) return true;
    if (y1 != rect.y1) return true;
    if (y2 != rect.y2) return true;
    return false;
  };

  T x1, y1, x2, y2;
private:
  inline static T clamp_range(T x, T l, T h) XBMC_FORCE_INLINE
  {
    return (x > h) ? h : ((x < l) ? l : x);
  }
};

typedef CPointGen<float> CPoint;
typedef CPointGen<int>   CPointInt;

typedef CShapeGen<float> CShape;
typedef CShapeGen<int>   CShapeInt;

typedef CCircleGen<float> CCircle;
typedef CCircleGen<int>   CCircleInt;

typedef CRectGen<float>  CRect;
typedef CRectGen<int>    CRectInt;
