#include "common.h"

// 全局变量定义
// 标定的Z焦距值列表，当前 Z 焦距值，下标值(combox对应)
// 初始 PTZ 变量
// 文件路径
// HSV检测算法值 
std::vector<int> g_ZValueLists;  // Z 不同焦距，根据标定文件读取
size_t g_zIdxL, g_zIdxR;
std::vector<std::vector<double>> g_initPTZState;  // 外层vector不同焦距时的初始状态， 内层vector是PL, TL, PR, TR
std::vector<std::vector<int>> g_initPTZStateHK;  
std::vector<std::vector< cv::Mat>> g_stereoParams; // 外层vector为不同焦距，最内层vector 为内外参

QString g_leftImgSavePath, g_rightImgSavePath, g_calibatePath, g_initPTZPath;
int g_LowH, g_HighH, g_LowS, g_HighS, g_LowV, g_HighV;
int g_kernalSize;
int g_lowCanny, g_HighCanny;

double g_RectifyScale;

// 4. 十六进制转十进制
double hkHex2Dec(int x) {
	char str[25];
	itoa(x, str, 16);
	return atoi(str) / 10.0;
}

// 5. 计算旋转矩阵，先绕x轴转alpha，再绕Y轴转beta
void getHKRotMatrix(double a, double b, cv::Mat& res) {
	//a = a < 180 ? a : 360 - a; // 欧拉变换逆时针为正，云台P左转边小，因此设置负号
	a = -a;
	a = a * PAI_DIV_180;
	b = -b * PAI_DIV_180;

	double sin_a = sin(a);
	double sin_b = sin(b);
	double cos_a = cos(a);
	double cos_b = cos(b);

	res = cv::Mat(3, 3, CV_64F);
	res.at<double>(0, 0) = cos_a;
	res.at<double>(0, 1) = 0;
	res.at<double>(0, 2) = sin_a;
	res.at<double>(1, 0) = -sin_a * sin_b;
	res.at<double>(1, 1) = cos_b;
	res.at<double>(1, 2) = cos_a * sin_b;
	res.at<double>(2, 0) = -sin_a * cos_b;
	res.at<double>(2, 1) = -sin_b;
	res.at<double>(2, 2) = cos_a * cos_b;

}

// 6. 深度图彩色显示
void _myConvertToColor(cv::Mat disp8, cv::Mat disp8u) {
	//把8位单通道图像转化为3通道彩色图像
	//Mat disp8u(disp8.rows, disp8.cols, CV_8UC3);
	for (int y = 0; y < disp8.rows; y++)
	{
		for (int x = 0; x < disp8.cols; x++)
		{
			uchar val = disp8.at<uchar>(y, x);
			uchar r, g, b;
			if (val == 0)
				r = g = b = 0;
			else
			{
				r = 255 - val;
				g = val < 128 ? val * 2 : (uchar)((255 - val) * 2);
				b = val;
			}
			disp8u.at<cv::Vec3b>(y, x) = cv::Vec3b(r, g, b);
		}
	}
}

// 7. 显示时间
void showCurrentTime(std::string str) {
	// 基于当前系统的当前日期/时间
	time_t now = time(0);
	tm *ltm = localtime(&now);
	std::cout << str << ": " << ltm->tm_sec << std::endl;
}

// 8. 漫水法
bool _areaCompare(const std::vector<int>& l, const std::vector<int>& r) { return l[1] > r[1]; };

bool _disCompare(const std::vector<int>& l, const std::vector<int>& r) { return l[3] < r[3]; };

double getDistance(cv::Point pointO, cv::Point pointA) {
	double distance;
	distance = powf((pointO.x - pointA.x), 2) + powf((pointO.y - pointA.y), 2);
	distance = sqrtf(distance);

	return distance;
};

cv::Vec3b RandomColor(int value) {
	value = value % 255;  //生成0~255的随机数
	cv::RNG rng;
	int aa = rng.uniform(0, value);
	int bb = rng.uniform(0, value);
	int cc = rng.uniform(0, value);
	return cv::Vec3b(aa, bb, cc);
};
