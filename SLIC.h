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
	Mat srcImage;		//�������ͼ��
	int numOfSuperpixels;		//�����صĸ���
	int numOfPixels;	//ͼ����������
	int step;		//������
	int step2;		//��������ƽ��

	Point2i* superpixels;	//�洢ÿһ�������ص�����
	int** labels;	//ÿһ����Ԫ[y,x]�洢���ص�(y,x)�����ľ�������
	int** distance2;	//ÿһ����Ԫ[y,x]�洢���ص�(y,x)����������ĵľ����ƽ�����𵽾�������ã�

	bool processed;		//ָʾ�Ƿ��ѽ��й�SLIC����

public:

	SLIC(Mat src);	//srcҪ��ΪBGR��ʽ

	SLIC(Mat src, int k);	//srcΪ������ͼ��BGR��ʽ����kΪԤ�ڳ�������
	Mat getResult();	//����BGR��ʽ�Ľ��ͼ��
	~SLIC();

private:
	void init(Mat src, int numOfSuperpixels);	//��ʼ������
	int calcDistance2(int x, int y, int superpixelIndex);	//�����(x,y)�볬����(superpixelIndex)֮��ľ����ƽ��
	void updatePixels();	//����ÿ�����ص�ľ������Ĺ����������
	bool updateClusteringCenter();	//���¾������ģ�����ֵָʾ���������Ƿ����ı�
	void doSLIC();	//����SLIC�㷨����

};


