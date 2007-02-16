/*
 * Copyright (C) 2001-2005 Ichiro Fujinaga, Michael Droettboom, and Karl MacMillan
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/* Derived from 
   Klette, R. and P. Zamperoni.  1996. Handbook of Image Processing Operators.
   New York: John Wiley & Sons.
*/

#ifndef kwm12032001_erode_dilate
#define kwm12032001_erode_dilate

#include <vector>
#include <algorithm>
#include "gamera.hpp"
#include "neighbor.hpp"
#include "image_utilities.hpp"
#include "vigra/distancetransform.hxx"

using namespace std;

namespace Gamera {

  
  /*
  * binary dilation with arbitrary structuring element
  */
  template<class T, class U>
  typename ImageFactory<T>::view_type* dilate_with_structure(const T &src, const U &structuring_element, Point origin, bool only_border=false)
  {
	typedef typename ImageFactory<T>::data_type data_type;
	typedef typename ImageFactory<T>::view_type view_type;
	typedef typename T::value_type value_type;
	int x,y;

	value_type blackval = black(src);

	data_type* dest_data = new data_type(src.size(), src.origin());
	view_type* dest = new view_type(*dest_data);

	// build list of structuring element offsets
	IntVector se_x;
	IntVector se_y;
	int left, right, top, bottom, xoff, yoff;
	left = right = top = bottom = 0;
	for (y = 0; y < (int)structuring_element.nrows(); y++)
		for (x = 0; x < (int)structuring_element.ncols(); x++)
		if (is_black(structuring_element.get(Point(x,y)))) {
			xoff = x - origin.x();
			yoff = y - origin.y();
			se_x.push_back(xoff);
			se_y.push_back(yoff);
			if (left < -xoff) left = -xoff;
			if (right < xoff) right = xoff;
			if (top < -yoff) top = -yoff;
			if (bottom < yoff) bottom = yoff;
		}

	// move structuring element over image and add its black pixels
	size_t i;
	int ncols = (int)src.ncols();
	int nrows = (int)src.nrows();
	int maxy = nrows - bottom;
	int maxx = ncols - right;
	// first skip borders for saving range checks
	for (y = top; y < maxy; y++)
		for (x = left; x < maxx; x++) {
		// is it a bulk pixel?
		if (only_border && x > 0 && x < ncols - 1 && y > 0 && y < nrows - 1 &&
			src.get(Point(x-1,y-1)) && src.get(Point(x,y-1)) &&
			src.get(Point(x+1,y-1)) && src.get(Point(x-1,y)) &&
			src.get(Point(x+1,y)) && src.get(Point(x-1,y+1)) &&
			src.get(Point(x,y+1)) && src.get(Point(x+1,y+1))) {
			dest->set(Point(x,y),blackval);
			continue;
		}
		if (is_black(src.get(Point(x,y)))) {
			for (i = 0; i < se_x.size(); i++) {
			dest->set(Point(x + se_x[i], y + se_y[i]), blackval);
			}
		}
		}
	// now process borders where structuring element leaves image
	int sx, sy;
	for (y = 0; y < nrows; y++)
		for (x = 0; x < ncols; x++)
		if (y < top || y >= maxy || x < left || x >= maxx) {
			if (is_black(src.get(Point(x,y)))) {
			for (i = 0; i < se_x.size(); i++) {
				sx = x + se_x[i];
				sy = y + se_y[i];
				if (sx >= 0 && sx < ncols && sy >= 0 && sy < nrows)
				dest->set(Point(sx, sy), blackval);
			}
			}
		}

	return dest;
  }


  /*
  * binary erosion with arbitrary structuring element
  */
  template<class T, class U>
  typename ImageFactory<T>::view_type* erode_with_structure(const T &src, const U &structuring_element, Point origin){
	typedef typename ImageFactory<T>::data_type data_type;
	typedef typename ImageFactory<T>::view_type view_type;
	typedef typename T::value_type value_type;
	int x,y;

	value_type blackval = black(src);

	data_type* dest_data = new data_type(src.size(), src.origin());
	view_type* dest = new view_type(*dest_data);

	// build list of structuring element offsets
	IntVector se_x;
	IntVector se_y;
	int left, right, top, bottom, xoff, yoff;
	left = right = top = bottom = 0;
	for (y = 0; y < (int)structuring_element.nrows(); y++)
		for (x = 0; x < (int)structuring_element.ncols(); x++)
		if (is_black(structuring_element.get(Point(x,y)))) {
			xoff = x - origin.x();
			yoff = y - origin.y();
			se_x.push_back(xoff);
			se_y.push_back(yoff);
			if (left < -xoff) left = -xoff;
			if (right < xoff) right = xoff;
			if (top < -yoff) top = -yoff;
			if (bottom < yoff) bottom = yoff;
		}

	// move structuring element over image and check whether it is
	// fully contained in the source image
	size_t i;
	bool contained;
	int maxy = (int)src.nrows() - bottom;
	int maxx = (int)src.ncols() - right;
	for (y = top; y < maxy; y++)
		for (x = left; x < maxx; x++) {
		if (is_black(src.get(Point(x,y)))) {
			contained = true;
			for (i = 0; i < se_x.size(); i++) {
			if (is_white(src.get(Point(x + se_x[i], y + se_y[i])))) {
				contained = false;
				break;
			}
			}
			if (contained) dest->set(Point(x,y),blackval);
		}
		}

	return dest;
  }

  template<class T>
  class Max {
  public:
    inline T operator() (typename vector<T>::iterator begin,
			 typename vector<T>::iterator end);
  };

  template<class T>
  inline T Max<T>::operator() (typename vector<T>::iterator begin,
			       typename vector<T>::iterator end) {
    return *(max_element(begin, end));
  }

  template<>
  inline OneBitPixel Max<OneBitPixel>::operator()
    (vector<OneBitPixel>::iterator begin,
     vector<OneBitPixel>::iterator end) {
    return *(min_element(begin, end));
  }

  template<class T>
  class Min {
  public:
    inline T operator() (typename vector<T>::iterator begin,
			 typename vector<T>::iterator end);
  };

  template<class T>
  inline T Min<T>::operator() (typename vector<T>::iterator begin,
			       typename vector<T>::iterator end) {
    return *(min_element(begin, end));
  }
  
  template<>
  inline OneBitPixel Min<OneBitPixel>::operator()
    (vector<OneBitPixel>::iterator begin,
     vector<OneBitPixel>::iterator end) {
    return *(max_element(begin, end));
  }
  /* in raw this functions was named erode_dilate() but for the template specialsation a workaround is needed to
  create no loop in calling with onebit images.*/
  template<class T>
  typename ImageFactory<T>::view_type* erode_dilate_original(T &m, const size_t times, int direction, int geo) {
    typedef typename ImageFactory<T>::data_type data_type;
    typedef typename ImageFactory<T>::view_type view_type;
    typedef typename T::value_type value_type;
    Max<value_type> max;
    Min<value_type> min;

    if (m.nrows() < 3 || m.ncols() < 3)
      return simple_image_copy(m);

    data_type* new_data = new data_type(m.size(), m.origin());
    view_type* new_view = new view_type(*new_data);

    try {
      if (times > 1) {
	view_type* flip_view = simple_image_copy(m);
	try {
	  unsigned int r, ngeo = 0;
	  bool n8;
	  ngeo = 1;
	  for (r = 1; r <= times; r++) {
	    if (r > 1) {
	      typename view_type::vec_iterator g = flip_view->vec_begin();
	      typename view_type::vec_iterator h = new_view->vec_begin();
	      for (; g != flip_view->vec_end(); g++, h++)
		*g = *h;
	    }
	    if (geo && (ngeo % 2 == 0))
	      n8 = true;
	    else
	      n8 = false;
	    if (direction) {
	      if (n8)
		neighbor4x(*flip_view, max, *new_view);
	      else
		neighbor9(*flip_view, max, *new_view);
	    }
	    else {
	      if (n8)
		neighbor4x(*flip_view, min, *new_view);
	      else
		neighbor9(*flip_view, min, *new_view);
	    }
	    ngeo++;
	  }
	} catch (std::exception e) {
	  delete flip_view->data();
	  delete flip_view;
	  throw;
	}
	delete flip_view->data();
	delete flip_view;
	return new_view;
      } else {
	if (direction) {
	  if (geo)
	    neighbor4x(m, max, *new_view);
	  else
	    neighbor9(m, max, *new_view);
	}
	else {
	  if (geo)
	    neighbor4x(m, min, *new_view);
	  else
	    neighbor9(m, min, *new_view);
	}
	return new_view;
      }
    } catch (std::exception e) {
      delete new_view;
      delete new_data;
      throw;
    }
  }
  
  //the  new erode_dilate function which calls the former original erode_dilate function
  template<class T>
  typename ImageFactory<T>::view_type* erode_dilate(T &m, const size_t times, int direction, int geo){
	typedef typename ImageFactory<T>::view_type view_type;
	view_type* new_view = new view_type();
	
	new_view = erode_dilate_original(m,times,direction,geo);
	return new_view;
  }
  
  /*template spceialsation of erode_dilate using erdode_with_structure/dilate_with_structure for onebit images in
  assumption that it should be faster*/
  template<>
  ImageFactory<OneBitImageView>::view_type* erode_dilate<OneBitImageView>(OneBitImageView &src, const size_t times, int direction, int geo){
	OneBitImageData* struct_data = new OneBitImageData(Dim(1+2*times,1+2*times),Point(0,0));
	OneBitImageView* struct_view = new OneBitImageView(*struct_data);
	OneBitImageView* result_view = new OneBitImageView();
	//creating structuring element
	for(int y = 0; y < (int)struct_view->nrows(); y++){
		for(int x = 0; x < (int)struct_view->ncols(); x++)
			struct_view->set(Point(x,y),OneBitPixel(1));
	}
	
	if(geo)
		result_view = erode_dilate_original(src,times,direction,geo);
	if(direction)
		result_view = erode_with_structure(src,*struct_view, Point(times,times));
	else
		result_view = dilate_with_structure(src,*struct_view,Point(times,times),false);

	return result_view;
  }
  
  template<class T>
  void erode(T& image) {
    erode_dilate(image, 1, 1, 0);
  }

  template<class T>
  void dilate(T& image) {
    erode_dilate(image, 1, 0, 0);
  }

  template<class T>
  class Rank {
    unsigned int rank;
  public:
    Rank<T>(unsigned int rank_) { rank = rank_ - 1; }
    inline T operator() (typename vector<T>::iterator begin,
			 typename vector<T>::iterator end);
  };

  template<class T>
  inline T Rank<T>::operator() (typename vector<T>::iterator begin,
				typename vector<T>::iterator end) {
    nth_element(begin, begin + rank, end);
    return *(begin + rank);
  }

  template<>
  inline OneBitPixel Rank<OneBitPixel>::operator() (vector<OneBitPixel>::iterator begin,
						    vector<OneBitPixel>::iterator end) {
    nth_element(begin, end - rank, end);
    return *(end - rank);
  }

  template<class T>
  typename ImageFactory<T>::view_type* rank(const T &m, unsigned int r) {
    typedef typename ImageFactory<T>::data_type data_type;
    typedef typename ImageFactory<T>::view_type view_type;
    if (m.nrows() < 3 || m.ncols() < 3)
      return simple_image_copy(m);

    data_type* new_data = new data_type(m.size(), m.origin());
    view_type* new_view = new view_type(*new_data);

    try {
      Rank<typename T::value_type> rank(r);
      neighbor9(m, rank, *new_view);
    } catch (std::exception e) {
      delete new_view;
      delete new_data;
      throw;
    }
    return new_view;
  }

  template<class T>
  class Mean {
  public:
    inline T operator() (typename vector<T>::iterator begin,
			 typename vector<T>::iterator end);
  };

  template<class T>
  inline T Mean<T>::operator() (typename vector<T>::iterator begin,
				typename vector<T>::iterator end) {
    long sum = 0;
    size_t size = end - begin;
    for (; begin != end; ++begin)
      sum += size_t(*begin);
    return T(sum / size);
  }

  template<class T>
  class Mode {
  public:
    inline T operator() (typename vector<T>::iterator begin,
			 typename vector<T>::iterator end);
  };

  template<class T>
  inline T Mode<T>::operator() (typename vector<T>::iterator begin,
				typename vector<T>::iterator end) {
    std::map<T, size_t> votes;
    T max_value;
    size_t max_count = 0;
    for (; begin != end; ++begin) {
      votes[*begin]++;
      if (votes[*begin] > max_count) {
	max_count = votes[*begin];
	max_value = *begin;
      }
    }
    return max_value;
  }

  template<class T>
  typename ImageFactory<T>::view_type* mean(T &m) {
    typedef typename ImageFactory<T>::data_type data_type;
    typedef typename ImageFactory<T>::view_type view_type;
    if (m.nrows() < 3 || m.ncols() < 3)
      return simple_image_copy(m);

    data_type* new_data = new data_type(m.size(), m.origin());
    view_type* new_view = new view_type(*new_data);

    try {
      Mean<typename T::value_type> mean_op;
      neighbor9(m, mean_op, *new_view);
    } catch (std::exception e) {
      delete new_view;
      delete new_data;
      throw;
    }
    return new_view;
  }

  template<class T>
  class All {
  public:
    inline T operator() (typename vector<T>::iterator begin,
			 typename vector<T>::iterator end);
  };

  template<class T>
  inline T All<T>::operator() (typename vector<T>::iterator begin,
			       typename vector<T>::iterator end) {
    typename vector<T>::iterator middle = begin + 4;
    for (; begin != end; ++begin)
      if (begin != middle)
	if (is_black(*begin))
	  return *middle;
    return pixel_traits<T>::white();
  }

  template<class T>
  void despeckle_single_pixel(T &m) {
    typedef typename ImageFactory<T>::data_type data_type;
    typedef typename ImageFactory<T>::view_type view_type;
    data_type* new_data = new data_type(m.size(), m.origin());
    view_type* new_view = new view_type(*new_data);

    try {
      All<typename T::value_type> all_op;
      neighbor9(m, all_op, *new_view);
      
      typename T::vec_iterator g = m.vec_begin();
      typename view_type::vec_iterator h = new_view->vec_begin();
      for (; g != m.vec_end(); g++, h++)
	*g = *h;
    } catch (std::exception e) {
      delete new_view;
      delete new_data;
      throw;
    }
    return;
  }

  template<class T>
  void despeckle(T &m, size_t size) {
    if (m.nrows() < 3 || m.ncols() < 3)
      return;
    if (size == 1) {
      despeckle_single_pixel(m);
      return;
    }
    typedef typename T::value_type value_type;
    ImageData<value_type> mat_data(m.dim(), m.origin());
    ImageView<ImageData<value_type> > tmp(mat_data);

    typedef std::vector<Point> PixelQueue;
    PixelQueue pixel_queue;
    pixel_queue.reserve(size * 2);
    for (size_t r = 0; r < m.nrows(); ++r) {
      for (size_t c = 0; c < m.ncols(); ++c) {
	if (is_white(tmp.get(Point(c, r))) && is_black(m.get(Point(c, r)))) {
	  pixel_queue.clear();
	  pixel_queue.push_back(Point(c, r));
	  bool bail = false;
	  tmp.set(Point(c, r), 1);
	  for (size_t i = 0;
	       (i < pixel_queue.size()) && (pixel_queue.size() < size);
	       ++i) {
	    Point center = pixel_queue[i];
	    for (size_t r2 = (center.y()>0) ? center.y() - 1 : 0; 
		 r2 < std::min(center.y() + 2, m.nrows()); ++r2) {
	      for (size_t c2 = (center.x()>0) ? center.x() - 1 : 0; 
		   c2 < std::min(center.x() + 2, m.ncols()); ++c2) {
		if (is_black(m.get(Point(c2, r2))) && is_white(tmp.get(Point(c2, r2)))) {
		  tmp.set(Point(c2, r2), 1);
		  pixel_queue.push_back(Point(c2, r2));
		} else if (tmp.get(Point(c2, r2)) == 2) {
		  bail = true;
		  break;
		}
	      }
	      if (bail)
		break;
	    }
	    if (bail)
	      break;
	  }
	  if (!bail && pixel_queue.size() < size) {
	    for (typename PixelQueue::iterator i = pixel_queue.begin();
		 i != pixel_queue.end(); ++i) {
	      m.set(Point(i->x(), i->y()), white(m));
	    }
	  } else {
	    for (typename PixelQueue::iterator i = pixel_queue.begin();
		 i != pixel_queue.end(); ++i) {
	      tmp.set(Point(i->x(), i->y()), 2);
	    }
	  }
	}
      }
    }
  }

  template<class T>
  Image* distance_transform(const T& src, int norm) {
    FloatImageData* dest_data = new FloatImageData(src.size(), src.origin());
    FloatImageView* dest = new FloatImageView(*dest_data);
    
    try {
      vigra::distanceTransform(src_image_range(src), dest_image(*dest), 0, norm);
    } catch (std::exception e) {
      delete dest;
      delete dest_data;
      throw;
    }
    return dest;
  }
}
#endif

