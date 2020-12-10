#include "PointCloud.h"


int nRegions = 0;

PointCloud::PointCloud() : rPoints(NULL), vPoints(NULL), pFlag(NULL), pRegions(NULL), pAmp(NULL), pColor(NULL), prNum(0), pvNum(0), boundingBoxSize(0), centerPoint(glm::vec3(0.0f, 0.0f, 0.0f)) {
}
void PointCloud::init(GLfloat* raw, int num, double threshold) {
    prNum = num;
    rPoints = new GLfloat[num * 4];  
    vPoints = new GLfloat[num * 3];
    pColor = new GLfloat[num * 3];
    pAmp = new GLfloat[num];
    pFlag = new bool[num];
    pRegions = new int[num];

    memcpy(rPoints, raw, num * sizeof(GLfloat) * 4);
    pvNum = heatmap(raw, num, vPoints, pColor, pAmp, threshold);
    for (int i = 0; i < pvNum; i++) {
        pFlag[i] = true;
        ampMap[getPointStr(vPoints[i * 3], vPoints[i * 3 + 1], vPoints[i * 3 + 2])] = pAmp[i];
    }
    updateProperties();
}
void PointCloud::reset(double threshold) {
    pvNum = heatmap(rPoints, prNum, vPoints, pColor, pAmp, threshold);
    ampMap.clear();
    for (int i = 0; i < pvNum; i++) {
        pFlag[i] = true;
        ampMap[getPointStr(vPoints[i * 3], vPoints[i * 3 + 1], vPoints[i * 3 + 2])] = pAmp[i];
    }
    updateProperties();
}
void PointCloud::clearSonarNoise() {
    for (int i = 0; i < this->pvNum; i++) {
        GLfloat x = this->vPoints[i * 3];
        GLfloat y = this->vPoints[i * 3 + 1];
        GLfloat z = this->vPoints[i * 3 + 2];
        if (z <= 20 || (z >= 46.3) || (abs(x) >= 14 || abs(y) >= 14) || (abs(x) + abs(y) <= 2.0)) {
            pFlag[i] = false;
        }
    }
    transform();
}
void PointCloud::useBilateralFilter(double radius, double normal_radius) {
    int niter = 1;
    Octree octree;
    time_t start, end, globstart, globend;
    std::time(&globstart);
    double size = loadAndSortPoints(vPoints, pColor, pAmp, pvNum, octree, radius);
    std::time(&end);

    std::cout << "Parameters:" << std::endl;
    std::cout << "Radius: " << radius << std::endl;
    std::cout << "Number of iterations " << niter << "." << std::endl;
    std::cout << "Normal radius: " << normal_radius << "." << std::endl;
    std::cout << "Octree with depth " << octree.getDepth() << " created." << std::endl;
    std::cout << "Octree contains " << octree.getNpoints() << " points." << std::endl;
    std::cout << "The bounding box size is " << octree.getSize() << std::endl;
    std::cout << "Reading and sorting the points took " << difftime(end, globstart)
        << " s." << std::endl;

    std::cout << "Octree statistics" << std::endl;
    octree.printOctreeStat();

    //creating the bilateral filter
    std::time(&start);
    BilateralFilter bilateralfilter(&octree, radius, normal_radius, niter);
    std::time(&end);

    //bilateralfilter.applyBilateralFilter();
    bilateralfilter.parallelApplyBilateralFilter();
    OctreeNode* node = octree.getRoot();
    pvNum = 0;
    ampMap.clear();
    saveContent(node, octree, bilateralfilter.getSetIndex(), false);
}
void PointCloud::saveContent(OctreeNode* node, Octree& octree, unsigned int index, bool isoriented)
{

    if (node->getDepth() != 0)
    {
        for (int i = 0; i < 8; i++)
            if (node->getChild(i) != NULL)
                saveContent(node->getChild(i), octree, index, isoriented);
    }
    else if (node->getNpts(index) != 0)
    {
        Sample_deque::const_iterator iter;
        std::vector<double>::iterator pi;
        for (iter = node->points_begin(index);
            iter != node->points_end(index); ++iter)
        {
            const Sample& s = *iter;
            size_t sindex = s.index();
            vPoints[pvNum * 3] = s.x();
            vPoints[pvNum * 3 + 1] = s.y();
            vPoints[pvNum * 3 + 2] = s.z();

            pColor[pvNum * 3] = octree.getProperty(sindex, 0);
            pColor[pvNum * 3 + 1] = octree.getProperty(sindex, 1);
            pColor[pvNum * 3 + 2] = octree.getProperty(sindex, 2);
            pAmp[pvNum] = octree.getProperty(sindex, 3);
            ampMap[getPointStr(s.x(), s.y(), s.z())] = pAmp[pvNum];
            pFlag[pvNum++] = true;
        }
    }
}
void PointCloud::transform() {
    int prevNum = pvNum;
    pvNum = 0;
    for (int i = 0; i < prevNum; i++) {
        if (pFlag[i]) {
            memcpy(vPoints + pvNum * 3, vPoints + i * 3, sizeof(GLfloat) * 3);
            memcpy(pColor + pvNum * 3, pColor + i * 3, sizeof(GLfloat) * 3);
            pAmp[pvNum] = pAmp[i];
            pFlag[pvNum] = true;
            pRegions[pvNum++] = pRegions[i];
        }
        else {
            ampMap.erase(getPointStr(vPoints[i * 3], vPoints[i * 3 + 1], vPoints[i * 3 + 2]));
        }
    }
    updateProperties();
}
void PointCloud::segment(float radius, int thresh) {
    nRegions = 0;
    cout << "Segmentation begins\n";
    int k;                   // number of nearest neighbors
    int dim = 3;             // dimension
    double eps = 0;          // error bound
    int nPts = this->pvNum;  // actual number of data points
    ANNpointArray dataPts;   // data points
    ANNpoint queryPt;        // query point
    ANNidxArray nnIdx;       // near neighbor indices
    ANNdistArray dists;      // near neighbor distances
    ANNkd_tree* kdTree;      // serarch structure

    queryPt = annAllocPt(dim);
    cout << (int)&queryPt << endl;
    dataPts = annAllocPts(nPts, dim);

    for (int i = 0; i < nPts; i++) {
        dataPts[i][0] = this->vPoints[i * 3];
        dataPts[i][1] = this->vPoints[i * 3 + 1];
        dataPts[i][2] = this->vPoints[i * 3 + 2];
    }
    kdTree = new ANNkd_tree(dataPts, nPts, dim);

    queue<int> q;
    int* crowd = new int[nPts];
    bool* flag = new bool[nPts];
    for (int i = 0; i < nPts; i++) {
        flag[i] = false;
    }
    for (int i = 0; i < nPts; i++) {
        if (flag[i]) { continue; }
        int count = 0;
        q.push(i);
        flag[i] = true;
        while (!q.empty()) {
            int queryIdx = q.front();
            crowd[count++] = queryIdx;
            queryPt[0] = dataPts[queryIdx][0];
            queryPt[1] = dataPts[queryIdx][1];
            queryPt[2] = dataPts[queryIdx][2];

            k = kdTree->annkFRSearch(
                queryPt,
                radius,
                0,
                NULL,
                NULL,
                eps);
            nnIdx = new ANNidx[k];
            dists = new ANNdist[k];
            kdTree->annkFRSearch(
                queryPt,
                radius,
                k,
                nnIdx,
                dists,
                eps);
            for (int j = 0; j < k; j++) {
                if (!flag[nnIdx[j]]) {
                    q.push(nnIdx[j]);
                    flag[nnIdx[j]] = true;
                }
            }
            q.pop();

            delete[] dists;
            delete[] nnIdx;
        }
        cout << count << endl;
        if (count <= thresh) {
            for (int j = 0; j < count; j++) {
                pFlag[crowd[j]] = false;
                pRegions[crowd[j]] = -1;
            }
        }
        else {
            for (int j = 0; j < count; j++) {
                pRegions[crowd[j]] = nRegions;
            }
            nRegions++;
        }
    }


    delete[] flag;
    delete[] crowd;
    delete kdTree;
    annDeallocPt(queryPt);
    annDeallocPts(dataPts);
    annClose();

    transform();
    return;
}
void PointCloud::updateProperties() {
    GLfloat minx, maxx, miny, maxy, minz, maxz;
    minx = maxx = vPoints[0];
    miny = maxy = vPoints[1];
    minz = maxz = vPoints[2];
    for (int i = 0; i < pvNum * 3; i += 3) {
        minx = glm::min(vPoints[i], minx);
        maxx = glm::max(vPoints[i], maxx);
        miny = glm::min(vPoints[i + 1], miny);
        maxy = glm::max(vPoints[i + 1], maxy);
        minz = glm::min(vPoints[i + 2], minz);
        maxz = glm::max(vPoints[i + 2], maxz);
    }
    centerPoint = glm::vec3((minx + maxx) / 2, (miny + maxy) / 2, (minz + maxz) / 2);
    boundingBoxSize = max(max(maxx - minx, maxy - miny), maxz - minz);
}
PointCloud::~PointCloud() {
    delete[] vPoints;
    delete[] rPoints;
    delete[] pColor;
    delete pAmp;
    delete[] pFlag;
    delete[] pRegions;
}
double loadAndSortPoints(GLfloat* points, GLfloat* color, float* amp, int num, Octree& octree, double min_radius)
{
    int nprop = 4;
    deque<Sample> input_vertices;

    double xmin, ymin, zmin, xmax, ymax, zmax;
    xmin = xmax = points[0];
    ymin = ymax = points[1];
    zmin = zmax = points[2];

    for (int i = 0; i < num; i++)
    {
        double x = points[i * 3], y = points[i * 3 + 1], z = points[i * 3 + 2];
        Sample temp(x, y, z);
        temp.setIndex(i);
        input_vertices.push_back(temp);
        xmin = std::min(x, xmin);
        xmax = std::max(x, xmax);
        ymin = std::min(y, ymin);
        ymax = std::max(y, ymax);
        zmin = std::min(z, zmin);
        zmax = std::max(z, zmax);

        //reading properties
        octree.addProperty(nprop);
        octree.setProperty(i, 0, color[i * 3]);
        octree.setProperty(i, 1, color[i * 3 + 1]);
        octree.setProperty(i, 2, color[i * 3 + 2]);
        octree.setProperty(i, 3, amp[i]);
    }

    std::cout << input_vertices.size() << " points read" << std::endl;

    double lx = xmax - xmin;
    double ly = ymax - ymin;
    double lz = zmax - zmin;

    double size = std::max(lx, ly);
    size = std::max(lz, size);
    size = 1.1 * size;//loose bounding box around the object

    double margin;
    if (min_radius > 0)
    {
        unsigned int depth = (unsigned int)ceil(log2(size / (min_radius)));
        //adapting the bouding box size so that the smallest cell size is 
        //exactly 2*min_radius
        double adapted_size = pow2(depth) * min_radius;
        //margins of the bouding box around the object
        margin = 0.5 * (adapted_size - size);
        size = adapted_size;
        octree.setDepth(depth);
    }
    else 
    {
        margin = 0.05 * size; //margins of the bouding box around the object
    }

    double ox = xmin - margin;
    double oy = ymin - margin;
    double oz = zmin - margin;
    Point origin(ox, oy, oz);

    octree.initialize(origin, size);

    //add the points to the set with index 0 (initial set)
    octree.addInitialPoints(input_vertices.begin(), input_vertices.end());

    return size;
}
string getPointStr(float x, float y, float z) {
    stringstream ss;
    ss << std::fixed << std::setprecision(0) << x << y << z;
    return ss.str();
}