#include <cstdio>
#include <cstring>
#include <chrono>
#include <vector>
#include <cmath>

using namespace std;
using namespace std::chrono;

extern "C" void filter_4x8(const float* x, int w, int h, float* y, const float* f);
extern "C" void filter_4x8_avx(const float* x, int w, int h, float* y, const float* f);

const int WX = 4096;
const int HX = 2048;
const int WY = WX - 7;
const int HY = HX - 2;

static vector<float> X4k(WX*HX);

static void test4k(int times, const char* name, void filter4x8(const float* x, int w, int h, float* y, const float* f))
{
	vector<float> Y(WY*(HX - 3));
	const float F[4][8] =
	{
		{0, 1, 2, 1, 0, -1, -2, -1},
		{0, 0.01, 0.02, 0.01, 0, -0.01, -0.02, -0.01},
		{0, 100, 200, 100, 0, -100, -200, -100},
		{0, -1, -2, -1, 0, 1, 2, 1},
	};
	auto s = high_resolution_clock::now();
	for (int i = times; i --> 0;)
	{
		filter4x8(X4k.data(), WX, HX, Y.data(), (float*)F);
	}
	auto dt = duration<double>(high_resolution_clock::now() - s)/times;

	float max_rdif = 0;
	float avg_rdif = 0;
	for (size_t i = 0; i < Y.size(); i++)
	{
		const float EXPECTED = -1600.0 - 0.16;
		float rdif = fabs((EXPECTED - Y[i])/EXPECTED);
		avg_rdif += rdif;
		max_rdif = (rdif > max_rdif) ? rdif : max_rdif;
	}
	avg_rdif /= Y.size();

	printf("%s x%i %lf s, %lf %lf\n", name, times, dt.count(), avg_rdif, max_rdif);
}

static bool test_pick(void filter4x8(const float* x, int w, int h, float* y, const float* f))
{
	vector<float> Y(WY*(HX - 3));
	for (int i = 0; i < 4; i++)
	for (int j = 0; j < 8; j++)
	{
		float F[4][8] = {};
		F[i][j] = 1.0;
		filter4x8(X4k.data(), WX, HX, Y.data(), (float*)F);
		for (int row = 0; row < HX - 3; row++)
		for (int col = 0; col < HY; col++)
		{
			float x = X4k[(row + i)*WX + col + j];
			float y =  Y[row*WY + col];
			if (x != y)
			{
				printf("%i %i, %i %i, %lf %lf\n", row, col, i, j, x, y);
				return false;
			}
		}
	}
	return true;
}

static bool test_place(void filter4x8(const float* x, int w, int h, float* y, const float* f))
{
	float F[4][8];
	for (int i = 0; i < 32; i++)
	{
		((float*)F)[i] = i;
	}
	float y;
	for (int i = 0; i < 4; i++)
	for (int j = 0; j < 8; j++)
	{
		float vx[4][8] = {};
		memset(vx, 0, sizeof(vx));
		vx[i][j] = 1.0;
		filter4x8((float*)vx, 8, 4, &y, (float*)F);
		if (y != 8*i + j)
		{
			printf("%i %i, %lf\n", i, j, y);
			return false;
		}
	}
	return true;
}
int main(int argn, char* argv[])
{
	for (int row = 0; row < HX; row++)
	for (int col = 0; col < WX; col++)
	{
		X4k[WX*row + col] = WX*(row%256) + col;
	}
	int times = (argn > 1) ? atoi(argv[1]) : 100;

	test4k(times, "c", filter_4x8);
	test4k(times, "avx", filter_4x8_avx);
	test_pick(filter_4x8_avx);
	test_place(filter_4x8_avx);
	test_place(filter_4x8);
	return 0;
}
