#ifndef FILEIO_H
#define FILEIO_H
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

class FileIO {
public:
	FileIO();
	static void savePointsAsObj(const std::string dataPath, float* points, int* regions, int num);
    static float* openBinaryPointsFile(const std::string dataPath, int size);
	static void savePointsAsBinaryFile();
    static int getFileSize(const std::string dataPath);
};

FileIO::FileIO(){}

void FileIO::savePointsAsObj(const std::string dataPath, float* points, int* regions, int num){
    std::ofstream wf(dataPath, std::ios::out);
	for (int i = 0; i < num; i++) {
		wf << "v " << points[i * 3] << " " << points[i * 3 + 1] << " " << points[i * 3 + 2] << " " << regions[i] << endl;
	}
	wf.close();
}

int FileIO::getFileSize(const std::string dataPath) {
    std::ifstream rf(dataPath, std::ios::binary | std::ios::in);
    // calculate number of points
    rf.seekg(0, std::ios::end);
    std::streampos size = rf.tellg();
    rf.close();
    return size;
}
float* FileIO::openBinaryPointsFile(const std::string dataPath, int size) {
    float* points = new float[size];
    std::ifstream rf(dataPath, std::ios::binary | std::ios::in);
    rf.read((char*)points, size);
    rf.close();
    return points;
}

void FileIO::savePointsAsBinaryFile() {

}
#endif