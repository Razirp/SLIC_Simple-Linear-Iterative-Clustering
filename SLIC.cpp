#include "SLIC.h"

SLIC::SLIC(Mat src)
{
	init(src, 1024);		//Ĭ�ϳ�������Ϊ1024
}

SLIC::SLIC(Mat src, int k)
{
	init(src, k);
}

void SLIC::doSLIC()
{
	processed = true;
	const int maxIterateNums = 20;	//������������
	for (int i = 0; i < maxIterateNums; ++i)
	{
		updatePixels();
		bool changed = updateClusteringCenter();
		if (!changed)
		{	//���ǰ�����ε����ľ�������û�з����仯������Ϊ����������ֹ���˳�ѭ��
			break;
		}
	}
}


void SLIC::updatePixels()
{
	for (int i = 0; i < numOfSuperpixels; ++i)
	{	//����ÿ��������
		//ɨ������Χ2step��2step��Χ�ڵ����ص�
		for (int dx = -step; dx <= step; ++dx)
		{
			int nowX = superpixels[i].x + dx;	//��ǰɨ�赽�����ص��x����
			if (nowX < 0 || nowX >= srcImage.cols)
			{	//�������ͼ��߽磬������
				continue;
			}
			for (int dy = -step; dy <= step; ++dy)
			{
				int nowY = superpixels[i].y + dy;	//��ǰɨ�赽�����ص��y����
				if (nowY < 0 || nowY >= srcImage.rows)
				{	//�������ͼ��߽磬������
					continue;
				}
				//����ǰ���ص�Ϸ�����������²���
				int newDis2 = calcDistance2(nowX, nowY, i);	//���㵱ǰ���ص㵽��i�����������ĵľ���ƽ��
				if (newDis2 < distance2[nowY][nowX] || distance2[nowY][nowX] == -1)
				{	//����¾����С���߾���Ϊȱʡֵ(-1)������¾���ͱ�ǩlabel
					distance2[nowY][nowX] = newDis2;
					labels[nowY][nowX] = i;		//���¾�������Ϊi
				}
			}
		}
	}
}

bool SLIC::updateClusteringCenter()
{
	bool changed = false;	//ָʾ���������Ƿ����ı�
	//�������鲢��ʼ��Ϊ0
	int* sumCount = new int[numOfSuperpixels]();	//��¼ÿ�������е�ĸ���
	int* sumX = new int[numOfSuperpixels]();	//��¼ÿ�������е��x�����
	int* sumY = new int[numOfSuperpixels]();	//��¼ÿ�������е��y�����
	if (sumCount == NULL || sumX == NULL || sumY == NULL)
	{	//�����ڴ�ʧ�����������˳�
		exit(-1);
	}

	//����ͼ�񣬼�������������
	for (int y = 0; y < srcImage.rows; ++y)
	{
		for (int x = 0; x < srcImage.cols; ++x)
		{
			int nowCenter = labels[y][x];	//��ǰ���صľ�������
			++sumCount[nowCenter];
			sumX[nowCenter] += x;
			sumY[nowCenter] += y;
		}
	}
	//���¾������ĵ�����ֵ
	for (int i = 0; i < numOfSuperpixels; ++i)
	{
		//�µľ�����������ȡ���������е�������ƽ��ֵ
		int newX = sumX[i] / sumCount[i];
		int newY = sumY[i] / sumCount[i];
		if (newX != superpixels[i].x || newY != superpixels[i].y)
		{	//������ֵ�����仯
			superpixels[i] = Point2i(newX, newY);	//��������ֵ
			changed = true;		//ָʾ���������ѷ����ı�
		}
	}

	delete[] sumY;
	delete[] sumX;
	delete[] sumCount;
	return changed;
}

int SLIC::calcDistance2(int x, int y, int superpixelIndex)
{
	const int m2 = 1600;		//Ȩ�ص�ƽ��

	int dl = srcImage.at<Vec3b>(superpixels[superpixelIndex].y, superpixels[superpixelIndex].x).val[0] - srcImage.at<Vec3b>(y, x).val[0];
	int da = srcImage.at<Vec3b>(superpixels[superpixelIndex].y, superpixels[superpixelIndex].x).val[1] - srcImage.at<Vec3b>(y, x).val[1];
	int db = srcImage.at<Vec3b>(superpixels[superpixelIndex].y, superpixels[superpixelIndex].x).val[2] - srcImage.at<Vec3b>(y, x).val[2];
	int dx = superpixels[superpixelIndex].x - x;
	int dy = superpixels[superpixelIndex].y - y;

	int dc2 = dl * dl + da * da + db * db;	//dc��ƽ��
	int ds2 = dx * dx + dy * dy;	//ds��ƽ��

	int distance2 = dc2 + (ds2 * m2) / step2;	//���չ�ʽ��������ƽ��
	return distance2;
}

void SLIC::init(Mat src, int k)
{
	processed = false;
	cvtColor(src, srcImage, COLOR_BGR2Lab);	//��Դͼ��ת��ΪLab��ʽ�������srcImage�ֶ�
	this->numOfPixels = srcImage.rows * srcImage.cols;	//��ʼ��������
	this->step2 = numOfPixels / k;	//��ʼ�������ز�����ƽ��
	this->step = sqrt(step2);	//��ʼ�������ز���
	int numOfSuperpixels_row = round((double)srcImage.rows / step);	//����������ȡ���������벽���ı�ֵ���������룩
	int numOfSuperpixels_col = round((double)srcImage.cols / step);	//����������ȡ���������벽���ı�ֵ���������룩
	this->numOfSuperpixels = numOfSuperpixels_col * numOfSuperpixels_row;	//��ʼ����������

	//���������ڴ�
	labels = new int* [srcImage.rows];
	distance2 = new int* [srcImage.rows];
	for (int i = 0; i < srcImage.rows; ++i)
	{
		labels[i] = new int[srcImage.cols];
		distance2[i] = new int[srcImage.cols];
		//��ʼ��ÿһ�����Label��distanceΪ-1
		memset(labels[i], -1, srcImage.cols * sizeof(int));
		memset(distance2[i], -1, srcImage.cols * sizeof(int));
	}

	superpixels = new Point2i[this->numOfSuperpixels];
	if (labels == NULL || distance2 == NULL || superpixels== NULL)
	{	//�����ڴ�ʧ�����������˳�
		exit(-1);
	}
	
	//��ʼ������������
	for (int i = 0; i < numOfSuperpixels_row; ++i)
	{	//��ÿһ��������
		int rowNow = step / 2 + i * step;	//��ǰ�������еĳ����ص�������
		for (int j = 0; j < numOfSuperpixels_col; ++j)
		{
			superpixels[i * numOfSuperpixels_col + j] = Point2i(step / 2 + j * step, rowNow);		//�洢��ǰ�����ص����꣨��x��y��
		}
	}

}

Mat SLIC::getResult()
{
	if (!processed)
	{	//���δ������������SLIC����
		doSLIC();	
	}
	Mat resultImage = srcImage.clone();	//���ͼ

	//����ÿ�������Labƽ��ֵ
	int* sumCount = new int[numOfSuperpixels]();	//��¼ÿ�������е�ĸ���
	int* averL = new int[numOfSuperpixels]();	//��¼ÿ�������е��L��ƽ��ֵ
	int* averA = new int[numOfSuperpixels]();	//��¼ÿ�������е��a��ƽ��ֵ
	int* averB = new int[numOfSuperpixels]();	//��¼ÿ�������е��b��ƽ��ֵ
	if (sumCount == NULL || averL == NULL || averA == NULL || averB == NULL)
	{	//�����ڴ�ʧ�����������˳�
		exit(-1);
	}

	//����ͼ�񣬼��������ĸ���
	for (int y = 0; y < srcImage.rows; ++y)
	{	//�Ӻ�
		for (int x = 0; x < srcImage.cols; ++x)
		{
			int nowCenter = labels[y][x];	//��ǰ���صľ�������
			++sumCount[nowCenter];
			averL[nowCenter] += srcImage.at<Vec3b>(y, x)[0];
			averA[nowCenter] += srcImage.at<Vec3b>(y, x)[1];
			averB[nowCenter] += srcImage.at<Vec3b>(y, x)[2];
		}
	}
	for (int i = 0; i < numOfSuperpixels; ++i)
	{	//ȡƽ��
		averL[i] /= sumCount[i];
		averA[i] /= sumCount[i];
		averB[i] /= sumCount[i];
	}
	delete[] sumCount;

	//��ͼ����ÿ�����Labֵ����Ϊ����Ӧ�����Labֵ�ľ�ֵ�������߽���Ϊ����
	for (int y = 0; y < resultImage.rows; ++y)
	{
		for (int x = 0; x < resultImage.cols; ++x)
		{
			int nowCenter = labels[y][x];
			//����Ƿ�Ϊ�߽磬����ΧedgeWidth��edgeWidth�����ڷ���label�仯������Ϊ�Ǳ߽�
			bool edgeFlag = false;
			const int edgeWidth = 1;
			for (int dx = -edgeWidth; dx <= edgeWidth && !edgeFlag; dx++)
			{
				for (int dy = -edgeWidth; dy <= edgeWidth && !edgeFlag; ++dy)
				{
					int nowX = x + dx;	//��ǰɨ�赽�����ص��x����
					int nowY = y + dy;	//��ǰɨ�赽�����ص��y����
					if (nowY < 0 || nowY >= srcImage.rows)
					{	//�����ͼ���Ե������Ϊ�Ǳ߽�
						edgeFlag = true;
					}
					else if (labels[nowY][nowX] != nowCenter)
					{	//�����label�仯������Ϊ�Ǳ߽�
						edgeFlag = true;
					}
				}
			}
			if (edgeFlag)
			{	//����Ǳ�Ե���������ɫΪ����ɫ
				resultImage.at<Vec3b>(y, x)[0] = 255;
				resultImage.at<Vec3b>(y, x)[1] = 128;
				resultImage.at<Vec3b>(y, x)[2] = 128;
			}
			else
			{	//������Ǳ�Ե���������ɫΪ��Ӧ����Lab�ľ�ֵ
				resultImage.at<Vec3b>(y, x)[0] = averL[nowCenter];
				resultImage.at<Vec3b>(y, x)[1] = averA[nowCenter];
				resultImage.at<Vec3b>(y, x)[2] = averB[nowCenter];
			}
		}
	}
	delete[] averB;
	delete[] averA;
	delete[] averL;

	cvtColor(resultImage, resultImage, COLOR_Lab2BGR);	//�ı�ͼ��ΪBGR��ʽ
	return resultImage;
}

SLIC::~SLIC()
{
	srcImage.release();
	if (labels != NULL)
	{
		for (int i = 0; i < srcImage.rows; ++i)
		{
			if (labels[i] != NULL)
			{
				delete[] labels[i];
			}
		}
		delete[] labels;
	}
	if (distance2 != NULL)
	{
		for (int i = 0; i < srcImage.rows; ++i)
		{
			if (distance2[i] != NULL)
			{
				delete[] distance2[i];
			}
		}
		delete[] distance2;
	}
	if (superpixels != NULL)
	{
		delete[] superpixels;
	}
}