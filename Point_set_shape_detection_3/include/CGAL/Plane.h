#ifndef CGAL_EFFICIENT_RANSAC_PLANE_H
#define CGAL_EFFICIENT_RANSAC_PLANE_H

#include "Primitive.h"
#include <set>

namespace CGAL {

  namespace Efficient_ransac {

    template <typename Kernel, class inputDataType>
    class Plane : public Primitive_ab<Kernel, inputDataType>
    {
    public:
      typedef typename std::vector<inputDataType>::iterator inputIterator;
      typedef typename std::vector<inputDataType>::const_iterator InputConstIterator;
      typedef typename Kernel::FT FT;
      typedef typename Kernel::Point_3 Point;
      typedef typename Kernel::Point_2 Point_2d;
      typedef typename Kernel::Vector_3 Vector;
      typedef typename Kernel::Plane_3 Plane_3;
      Plane_3	m_plane;
      Point m_point_on_primitive;
      Vector m_base1, m_base2;

    public:
      Plane() :  Primitive_ab<Kernel, inputDataType>(0.1, 0.9) {m_type = PLANE; m_type_name ="Plane";}
      Plane(FT _a, FT _b) : Primitive_ab<Kernel, inputDataType>(_a, _b) {m_type = PLANE;m_type_name ="Plane";}
      void compute(std::set<int> &l_list_index_selected, Pwn_iterator &m_it_Point_Normal) 
      {
        m_isValid = true;
        if ( l_list_index_selected.size() < 3) {
          m_isValid = false; return;}

        std::vector<int> output(l_list_index_selected.begin(), l_list_index_selected.end()); 

        m_plane = Plane_3((m_it_Point_Normal + output[0])->first, 
          (m_it_Point_Normal + output[1])->first, 
          (m_it_Point_Normal + output[2])->first
          );


        //check deviation of the 3 normal
        Vector l_v;
        //FT cosTheta;
        for (int i=0;i<3;i++)
        {
          l_v = (m_it_Point_Normal + output[i])->second;
          //cosTheta= abs( l_v * m_plane.orthogonal_vector () )/sqrtf(l_v.squared_length()*m_plane.orthogonal_vector ().squared_length());
          //printf("val %d %f\n", i, cosTheta); 
          //if ( cosTheta < m_normalThresh)  m_isValid = false;
          if (abs(l_v * m_plane.orthogonal_vector () ) < m_normalThresh * sqrt(l_v.squared_length() * m_plane.orthogonal_vector ().squared_length())) {
            m_isValid = false;
            return;
          }

          m_point_on_primitive = (m_it_Point_Normal + output[0])->first;
          m_base1 = m_plane.base1();
          m_base1 = m_base1 * (1.0 / sqrt(m_base1.squared_length()));

          m_base2 = m_plane.base2();
          m_base2 = m_base2 * (1.0 / sqrt(m_base2.squared_length()));
        }
      }
      operator Plane_3 () { return m_plane;}
      std::string info()
      {
        std::stringstream sstr;
        Vector n = m_plane.orthogonal_vector();
        n = n * (1.0 / sqrt(n.squared_length()));
        FT d = (CGAL::ORIGIN - m_point_on_primitive) * n;
        sstr << "Type: " << m_type_name << "(" << n.x() << ", " << n.y() << ", " << n.z() << ")x - " << d << "= 0"
          << " ev: " << ExpectedValue() << " s: " << m_nb_subset_used << " #Pts: " <<  m_indices.size()	<< std::endl;

        return sstr.str();
      };
      std::string type_str() const {return m_type_name;}

      Point pointOnPrimitive() const {/*return m_plane.point();*/return m_point_on_primitive;}
/*
      Point projection() const {return projection(pointOnPrimitive()) ;}
      Point projection(const Point &_p) const {return m_plane.projection (_p);}*/

      void parameters(InputConstIterator first, std::vector<std::pair<FT, FT>> &parameterSpace, const std::vector<int> &indices) const {
        for (unsigned int i = 0;i<indices.size();i++) {
          Vector p = (first[indices[i]].first - m_plane.point());
          FT u = p * m_base1;
          FT v = p * m_base2;
          parameterSpace[i] = std::pair<FT, FT>(u, v);
        }
      }

      void parameterExtend(const Point &center, FT width, FT min[2], FT max[2]) const {
        min[0] = min[1] = max[0] = max[1] = 0;
        Point corner[8];
        corner[0] = center + Vector(width, width, width);
        corner[1] = center + Vector(-width, width, width);
        corner[2] = center + Vector(width, -width, width);
        corner[3] = center + Vector(-width, -width, width);
        corner[4] = center + Vector(width, width, -width);
        corner[5] = center + Vector(-width, width, -width);
        corner[6] = center + Vector(width, -width, -width);
        corner[7] = center + Vector(-width, -width, -width);

        for (unsigned int i = 0;i<8;i++) {
          Vector p = (corner[i] - m_plane.point());
          FT u = p * m_base1;
          FT v = p * m_base2;
          min[0] = std::min(min[0], u);
          min[1] = std::min(min[1], v);
          max[0] = std::max(max[0], u);
          max[1] = std::max(max[1], v);
        }
      }

      FT squared_distance(const Point &_p) const {return CGAL::squared_distance ( m_plane, _p);}

      void squared_distance(InputConstIterator first, std::vector<FT> &dists, const std::vector<int> &shapeIndex, const std::vector<unsigned int> &indices) {
        for (unsigned int i = 0;i<indices.size();i++) {
          if (shapeIndex[indices[i]] == -1)
            dists[i] = CGAL::squared_distance(m_plane, first[indices[i]].first);
        }
      }

      void cos_to_normal(InputConstIterator first, std::vector<FT> &angles, const std::vector<int> &shapeIndex, const std::vector<unsigned int> &indices) const {
        Vector n = m_plane.orthogonal_vector();
        n = n / (sqrt(n.squared_length()));
        for (unsigned int i = 0;i<indices.size();i++) {
          if (shapeIndex[indices[i]] == -1)
            angles[i] = abs(first[indices[i]].second * n);
        }
      }

      FT cos_to_normal(const Point &_p, const Vector &_n) const{return abs(_n * m_plane.orthogonal_vector());} 

      virtual bool supportsConnectedComponent() {return true;}
      virtual bool wrapsU() const {return false;}
      virtual bool wrapsV() const {return false;}
    };
  }
}
#endif
