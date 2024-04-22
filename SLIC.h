#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#include <cmath>
#include <cstring>

using namespace std;
using namespace cv;

class SLIC
{
private:
	Mat srcImage;		//待处理的图像
	int numOfSuperpixels;		//超像素的个数
	int numOfPixels;	//图像像素总数
	int step;		//网格间隔
	int step2;		//网格间隔的平方

	Point2i* superpixels;	//存储每一个超像素的坐标
	int** labels;	//每一个单元[y,x]存储像素点(y,x)所属的聚类中心
	int** distance2;	//每一个单元[y,x]存储像素点(y,x)到其聚类中心的距离的平方（起到距离的作用）

	bool processed;		//指示是否已进行过SLIC处理

public:

	SLIC(Mat src);	//src要求为BGR格式

	SLIC(Mat src, int k);	//src为待处理图像（BGR格式），k为预期超像素数
	Mat getResult();	//返回BGR格式的结果图像
	~SLIC();

private:
	void init(Mat src, int numOfSuperpixels);	//初始化函数
	int calcDistance2(int x, int y, int superpixelIndex);	//计算点(x,y)与超像素(superpixelIndex)之间的距离的平方
	void updatePixels();	//更新每个像素点的聚类中心归属及其距离
	bool updateClusteringCenter();	//更新聚类中心，返回值指示聚类中心是否发生改变
	void doSLIC();	//进行SLIC算法处理

};


