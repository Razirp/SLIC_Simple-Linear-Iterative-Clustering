// main.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
#include "SLIC.h"

int main(int argc, char* argv[])
{
    cout << "请输入图片路径(输入0表示取缺省图片路径）：" << endl;
    string url;
    cin >> url;
    if (url == "0")
    {
        url = "./images/63536f2f01409f750162828a980a0380_720w.jpg";
    }
    Mat image = imread(url, IMREAD_COLOR);
    if (image.empty())
    {   
        fprintf(stderr, "Can't load image!\n");
        return -1;
    }
    cout << "请输入预期超像素个数k（输入0或负数则表示取缺省值）：" << endl;
    int k;
    cin >> k;
    if (k <= 0)
    {
        k = 1024; //k缺省值为1024
    }
    
    SLIC slic(image, k);
    Mat resultImage = slic.getResult();

    imwrite("./result.jpg", resultImage);   //保存结果图像
    //显示原图和结果图
    namedWindow("原图", WINDOW_AUTOSIZE);
    namedWindow("SLIC结果图", WINDOW_AUTOSIZE);
    imshow("原图", image);
    imshow("SLIC结果图", resultImage);
    waitKey(0);
    image.release();
    resultImage.release();
    return 0;
}