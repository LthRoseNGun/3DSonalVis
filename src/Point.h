/**
 * @file Point.h
 * @brief Declares a generic point class
 * @author Julie Digne
 * @date 2012/10/10
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

#ifndef POINT_H
#define POINT_H


/**
 * @class Point
 * @brief Generic unoriented point
 *
 * The most standard 3D point structure: only contains three coordinates
 */
class Point
{
    public :
        /** @brief default constructor
         */
        Point() {
            m_x = m_y = m_z = 0;
        }

        /** @brief constructor
         * @param x x coordinate
         * @param y y coordinate
         * @param z z coordinate
         **/
        Point(double x, double y, double z) {
            m_x = x;
            m_y = y;
            m_z = z;
        }


        /** @brief access x coordinate
         * @return x
         */
        double x() const {
            return m_x;
        }

        /** @brief access y coordinate
         * @return y
         */
        double y() const {
            return m_y;
        }

        /** @brief access y coordinate
         * @return y
         */
        double z() const {
            return m_z;
        }


    private :
        
        /** @brief x coordinate*/
        double m_x;
        
        /** @brief y coordinate*/
        double m_y;
        
        /** @brief z coordinate*/
        double m_z;

};

#endif