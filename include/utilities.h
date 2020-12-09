/**
 * @file utilities.h
 * @brief defines some useful functions
 * @author Julie Digne
 * @date 2012/10/25
 * @copyright This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published 
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef UTILITIES_H
#define UTILITIES_H

#include<cstdlib>
#include<set>
#include<cmath>

#include "Point.h"
#include "ColorGradient.h"

/** @brief get the common element of two sets, if any
 * @param set1 first set to compare
 * @param set2 second set to compare
 * @return common element if any, NULL if none
 */
template <class T>
T getCommonElement(const std::set<T> &set1, const std::set<T> &set2)
{
    if(set1.empty() || set2.empty()) return NULL;
    
    typename std::set<T>::const_iterator it1 = set1.begin(); 
    typename std::set<T>::const_iterator it1End = set1.end();
    typename std::set<T>::const_iterator it2 = set2.begin(); 
    typename std::set<T>::const_iterator it2End = set2.end();
    
    if(*it1 > *set2.rbegin() || *it2 > *set1.rbegin()) return NULL;
    
    while(it1 != it1End && it2 != it2End)
    {
        if(*it1 == *it2) return *it1;
        if(*it1 < *it2) { ++it1; }
        else { ++it2; }
    }
    
    return NULL;
}

/** @brief compute 2 to the n
 * @param n power of 2
 * @return 2^n
 */
inline static int pow2(int n)
{
    unsigned int x=1<<n;
    return (int)x;
}

/** @brief compute the square distance between two points
 * @param p1 first point
 * @param p2 second point
 * @return square distance
 */
inline static double dist2(const Point &p1, const Point &p2)
{
    return ((p1.x() - p2.x()) * (p1.x() - p2.x())
    +(p1.y() - p2.y()) * (p1.y() - p2.y())
    +(p1.z() - p2.z()) * (p1.z() - p2.z()));
}

/** @brief compute the midpoint between two points
 * @param p1 first point
 * @param p2 second point
 * @return midpoint
 */
inline static Point midpoint(const Point &p1,const Point &p2)
{
    return Point( 0.5 * (p1.x() + p2.x()),
                  0.5 * (p1.y() + p2.y()),
                  0.5 * (p1.z() + p2.z()));
}

/** @brief compute the cross product of two vectors
 * @param v1x first vector x component
 * @param v1y first vector y component
 * @param v1z first vector z component
 * @param v2x second vector x component
 * @param v2y second vector y component
 * @param v2z second vector z component
 * @param resx resulting vector x component
 * @param resy resulting vector y component
 * @param resz resulting vector z component
 */
inline static void cross_product(double v1x, double v1y, double v1z, 
                                 double v2x, double v2y, double v2z,
                                 double &resx, double &resy, double &resz)
{
    resx =   v1y * v2z - v1z * v2y;
    resy = - v1x * v2z + v1z * v2x;
    resz =   v1x * v2y - v1y * v2x; 
}

/** @brief normalize vector components so that the vector as norm $1$
 * @param vx x component
 * @param vy y component
 * @param vz z component
 */
inline static void normalize(double &vx, double &vy, double &vz)
{
    double t = 1./ sqrt(vx * vx + vy * vy + vz * vz);
    vx = vx*t;
    vy = vy*t;
    vz = vz*t;
}

inline float* equalizeHist(float* amp, int num, float amp_min, float amp_max) {
    int hist[256] = { 0 }, new_hist[256] = { 0 };
    for (int i = 0; i < num; i++) {
        hist[int(255.0f * (amp[i] - amp_min) / (amp_max - amp_min))]++;
    }
    int curr = 0;
    for (int i = 0; i < 256; i++) {
        curr += hist[i];
        new_hist[i] = round(curr * 255.0f / num);
    }
    float* histAmp = new float[num];
    for (int i = 0; i < num; i++) {
        histAmp[i] = new_hist[int(255.0f * (amp[i] - amp_min) / (amp_max - amp_min))] / 255.0f;
    }
    return histAmp;
}

inline int heatmap(float* raw, int num, float* &points, float* &color, float* &amp, float threshold) {
    ColorGradient colorGradient = ColorGradient();

    float amp_min, amp_max = raw[3];
    for (int i = 0; i < num; i++) {
        amp_max = max(amp_max, raw[i * 4 + 3]);
    }
    float amp_threshold = amp_max * threshold;
    int j = 0;
    amp_min = amp_max;
    for (int i = 0; i < num; i++) {
        if (raw[i * 4 + 3] < amp_threshold) { continue; }
        memcpy(points + j * 3, raw + i * 4, 3 * sizeof(float));
        amp[j] = raw[i * 4 + 3];
        amp_min = min(amp_min, amp[j]);
        j++;
    }
    float* histAmp = equalizeHist(amp, j, amp_min, amp_max);
    for (int i = 0; i < j; i++) {
        colorGradient.getColorAtValue(histAmp[i], color[i * 3], color[i * 3 + 1], color[i * 3 + 2]);
    }
    delete[] histAmp;
    return j;
}


#endif
