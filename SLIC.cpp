#include "SLIC.h"

SLIC::SLIC(Mat src)
{
	init(src, 1024);		//默认超像素数为1024
}

SLIC::SLIC(Mat src, int k)
{
	init(src, k);
}

void SLIC::doSLIC()
{
	processed = true;
	const int maxIterateNums = 20;	//设置最大迭代数
	for (int i = 0; i < maxIterateNums; ++i)
	{
		updatePixels();
		bool changed = updateClusteringCenter();
		if (!changed)
		{	//如果前后两次迭代的聚类中心没有发生变化，则认为迭代收敛终止，退出循环
			break;
		}
	}
}


void SLIC::updatePixels()
{
	for (int i = 0; i < numOfSuperpixels; ++i)
	{	//对于每个超像素
		//扫描其周围2step×2step范围内的像素点
		for (int dx = -step; dx <= step; ++dx)
		{
			int nowX = superpixels[i].x + dx;	//当前扫描到的像素点的x坐标
			if (nowX < 0 || nowX >= srcImage.cols)
			{	//如果超出图像边界，则跳过
				continue;
			}
			for (int dy = -step; dy <= step; ++dy)
			{
				int nowY = superpixels[i].y + dy;	//当前扫描到的像素点的y坐标
				if (nowY < 0 || nowY >= srcImage.rows)
				{	//如果超出图像边界，则跳过
					continue;
				}
				//若当前像素点合法，则进行以下操作
				int newDis2 = calcDistance2(nowX, nowY, i);	//计算当前像素点到第i个超像素中心的距离平方
				if (newDis2 < distance2[nowY][nowX] || distance2[nowY][nowX] == -1)
				{	//如果新距离更小或者距离为缺省值(-1)，则更新距离和标签label
					distance2[nowY][nowX] = newDis2;
					labels[nowY][nowX] = i;		//更新聚类中心为i
				}
			}
		}
	}
}

bool SLIC::updateClusteringCenter()
{
	bool changed = false;	//指示聚类中心是否发生改变
	//申请数组并初始化为0
	int* sumCount = new int[numOfSuperpixels]();	//记录每个聚类中点的个数
	int* sumX = new int[numOfSuperpixels]();	//记录每个聚类中点的x坐标和
	int* sumY = new int[numOfSuperpixels]();	//记录每个聚类中点的y坐标和
	if (sumCount == NULL || sumX == NULL || sumY == NULL)
	{	//分配内存失败则程序故障退出
		exit(-1);
	}

	//遍历图像，计算上述三个量
	for (int y = 0; y < srcImage.rows; ++y)
	{
		for (int x = 0; x < srcImage.cols; ++x)
		{
			int nowCenter = labels[y][x];	//当前像素的聚类中心
			++sumCount[nowCenter];
			sumX[nowCenter] += x;
			sumY[nowCenter] += y;
		}
	}
	//更新聚类中心的坐标值
	for (int i = 0; i < numOfSuperpixels; ++i)
	{
		//新的聚类中心坐标取聚类中所有点的坐标的平均值
		int newX = sumX[i] / sumCount[i];
		int newY = sumY[i] / sumCount[i];
		if (newX != superpixels[i].x || newY != superpixels[i].y)
		{	//若坐标值发生变化
			superpixels[i] = Point2i(newX, newY);	//更新坐标值
			changed = true;		//指示聚类中心已发生改变
		}
	}

	delete[] sumY;
	delete[] sumX;
	delete[] sumCount;
	return changed;
}

int SLIC::calcDistance2(int x, int y, int superpixelIndex)
{
	const int m2 = 1600;		//权重的平方

	int dl = srcImage.at<Vec3b>(superpixels[superpixelIndex].y, superpixels[superpixelIndex].x).val[0] - srcImage.at<Vec3b>(y, x).val[0];
	int da = srcImage.at<Vec3b>(superpixels[superpixelIndex].y, superpixels[superpixelIndex].x).val[1] - srcImage.at<Vec3b>(y, x).val[1];
	int db = srcImage.at<Vec3b>(superpixels[superpixelIndex].y, superpixels[superpixelIndex].x).val[2] - srcImage.at<Vec3b>(y, x).val[2];
	int dx = superpixels[superpixelIndex].x - x;
	int dy = superpixels[superpixelIndex].y - y;

	int dc2 = dl * dl + da * da + db * db;	//dc的平方
	int ds2 = dx * dx + dy * dy;	//ds的平方

	int distance2 = dc2 + (ds2 * m2) / step2;	//依照公式计算距离的平方
	return distance2;
}

void SLIC::init(Mat src, int k)
{
	processed = false;
	cvtColor(src, srcImage, COLOR_BGR2Lab);	//将源图像转化为Lab格式并分配给srcImage字段
	this->numOfPixels = srcImage.rows * srcImage.cols;	//初始化像素数
	this->step2 = numOfPixels / k;	//初始化超像素步长的平方
	this->step = sqrt(step2);	//初始化超像素步长
	int numOfSuperpixels_row = round((double)srcImage.rows / step);	//超像素行数取行像素数与步长的比值（四舍五入）
	int numOfSuperpixels_col = round((double)srcImage.cols / step);	//超像素列数取列像素数与步长的比值（四舍五入）
	this->numOfSuperpixels = numOfSuperpixels_col * numOfSuperpixels_row;	//初始化超像素数

	//申请数组内存
	labels = new int* [srcImage.rows];
	distance2 = new int* [srcImage.rows];
	for (int i = 0; i < srcImage.rows; ++i)
	{
		labels[i] = new int[srcImage.cols];
		distance2[i] = new int[srcImage.cols];
		//初始化每一个点的Label和distance为-1
		memset(labels[i], -1, srcImage.cols * sizeof(int));
		memset(distance2[i], -1, srcImage.cols * sizeof(int));
	}

	superpixels = new Point2i[this->numOfSuperpixels];
	if (labels == NULL || distance2 == NULL || superpixels== NULL)
	{	//分配内存失败则程序故障退出
		exit(-1);
	}
	
	//初始化超像素坐标
	for (int i = 0; i < numOfSuperpixels_row; ++i)
	{	//对每一超像素行
		int rowNow = step / 2 + i * step;	//当前超像素行的超像素的行坐标
		for (int j = 0; j < numOfSuperpixels_col; ++j)
		{
			superpixels[i * numOfSuperpixels_col + j] = Point2i(step / 2 + j * step, rowNow);		//存储当前超像素的坐标（横x纵y）
		}
	}

}

Mat SLIC::getResult()
{
	if (!processed)
	{	//如果未处理过，则进行SLIC处理
		doSLIC();	
	}
	Mat resultImage = srcImage.clone();	//结果图

	//计算每个聚类的Lab平均值
	int* sumCount = new int[numOfSuperpixels]();	//记录每个聚类中点的个数
	int* averL = new int[numOfSuperpixels]();	//记录每个聚类中点的L的平均值
	int* averA = new int[numOfSuperpixels]();	//记录每个聚类中点的a的平均值
	int* averB = new int[numOfSuperpixels]();	//记录每个聚类中点的b的平均值
	if (sumCount == NULL || averL == NULL || averA == NULL || averB == NULL)
	{	//分配内存失败则程序故障退出
		exit(-1);
	}

	//遍历图像，计算上述四个量
	for (int y = 0; y < srcImage.rows; ++y)
	{	//加和
		for (int x = 0; x < srcImage.cols; ++x)
		{
			int nowCenter = labels[y][x];	//当前像素的聚类中心
			++sumCount[nowCenter];
			averL[nowCenter] += srcImage.at<Vec3b>(y, x)[0];
			averA[nowCenter] += srcImage.at<Vec3b>(y, x)[1];
			averB[nowCenter] += srcImage.at<Vec3b>(y, x)[2];
		}
	}
	for (int i = 0; i < numOfSuperpixels; ++i)
	{	//取平均
		averL[i] /= sumCount[i];
		averA[i] /= sumCount[i];
		averB[i] /= sumCount[i];
	}
	delete[] sumCount;

	//将图像中每个点的Lab值更改为其相应聚类的Lab值的均值，并将边界设为纯白
	for (int y = 0; y < resultImage.rows; ++y)
	{
		for (int x = 0; x < resultImage.cols; ++x)
		{
			int nowCenter = labels[y][x];
			//检测是否为边界，若周围edgeWidth×edgeWidth邻域内发生label变化，则认为是边界
			bool edgeFlag = false;
			const int edgeWidth = 1;
			for (int dx = -edgeWidth; dx <= edgeWidth && !edgeFlag; dx++)
			{
				for (int dy = -edgeWidth; dy <= edgeWidth && !edgeFlag; ++dy)
				{
					int nowX = x + dx;	//当前扫描到的像素点的x坐标
					int nowY = y + dy;	//当前扫描到的像素点的y坐标
					if (nowY < 0 || nowY >= srcImage.rows)
					{	//如果在图像边缘，则认为是边界
						edgeFlag = true;
					}
					else if (labels[nowY][nowX] != nowCenter)
					{	//如果有label变化，则认为是边界
						edgeFlag = true;
					}
				}
			}
			if (edgeFlag)
			{	//如果是边缘，则更新颜色为纯白色
				resultImage.at<Vec3b>(y, x)[0] = 255;
				resultImage.at<Vec3b>(y, x)[1] = 128;
				resultImage.at<Vec3b>(y, x)[2] = 128;
			}
			else
			{	//如果不是边缘，则更新颜色为相应聚类Lab的均值
				resultImage.at<Vec3b>(y, x)[0] = averL[nowCenter];
				resultImage.at<Vec3b>(y, x)[1] = averA[nowCenter];
				resultImage.at<Vec3b>(y, x)[2] = averB[nowCenter];
			}
		}
	}
	delete[] averB;
	delete[] averA;
	delete[] averL;

	cvtColor(resultImage, resultImage, COLOR_Lab2BGR);	//改变图像为BGR格式
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