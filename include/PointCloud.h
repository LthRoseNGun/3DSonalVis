#ifndef POINT_CLOUD_H
#define POINT_CLOUD_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <queue>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <ANN/ANN.h>


#include "BilateralFilter.h"
#include "types.h"
#include "Octree.h"
#include "Sample.h"
#include "utilities.h"
#include <deque>
#include <ctime>
#include <unordered_map>
#include <string>
#include <iomanip>

using namespace std;

double loadAndSortPoints(GLfloat* points, GLfloat* color, float* amp, int num, Octree& octree, double min_radius);
string getPointStr(float x, float y, float z);
class PointCloud {
public:
    GLfloat* rPoints;
    GLfloat* vPoints;
    GLfloat* pColor;
    float* pAmp;
    bool* pFlag;
    int* pRegions;
    int prNum, pvNum;
    float boundingBoxSize;
    glm::vec3 centerPoint;
    unordered_map<string, float> ampMap;
    PointCloud();
    void init(GLfloat* raw, int num, double threshold);
    void reset(double threshold);
    void clearSonarNoise();
    void useBilateralFilter(double radius = 0.1, double normal_radius = 0.1);
    void saveContent(OctreeNode* node, Octree& octree, unsigned int index, bool isoriented);
    void transform();
    void segment(float radius, int thresh = 100);
    void updateProperties();
    ~PointCloud();
};


#endif