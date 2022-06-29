#ifdef __cplusplus
extern "C" {
#endif

void filter_4x8(const float* __restrict__ x, int w, int h, float* __restrict__ y, const float* __restrict__ f)
{
	int wy = w - 7;
	for (int row = 0; row < h - 3; row++)
	for (int col = 0; col < w - 7; col++)
	{
		y[row*wy + col] = f[0]*x[row*w + col];
		for (int i = 0; i < 4; i++)
		for (int j = !i; j < 8; j++)
		{
			y[row*wy + col] += f[8*i + j]*x[(row + i)*w + col + j];
		}
	}
}

void filter_4x8_avx(const float* __restrict__ x, int w, int h, float* __restrict__ y, const float* __restrict__ f)
{
	float vf[4][8];
	for (int row = 0; row < 4; row++)
	for (int col = 0; col < 8; col++)
	{
		vf[row][col] = f[8*(row + 1) - col - 1];
	}
	__asm__ __volatile__ (
	".macro reaver write;"
		"vbroadcastss ymm0, [%[x]];"
		"vbroadcastss ymm1, [%[x] + %[w]];"
		"vbroadcastss ymm2, [%[x] + 2*%[w]];"
		"add %[x], %[w];"
		"add %[ofs], 4;"
		"vmulps ymm0, ymm0, ymm4;"
		"vmulps ymm1, ymm1, ymm5;"
		"vaddps ymm1, ymm1, ymm0;"
		"vbroadcastss ymm0, [%[x] + 2*%[w]];"
		"vmulps ymm0, ymm0, ymm7;"
		"vmulps ymm2, ymm2, ymm6;"
		"vaddps ymm2, ymm2, ymm1;"
		"vaddps ymm2, ymm2, ymm0;"
		"sub %[x], %[w];"
		"add %[x], 4;"
		"vaddps ymm3, ymm3, ymm2;"
		".if \\write;"
			"vmovss [%[y]], xmm3;"
		".endif;"
		"vpermilps ymm3, ymm3, 0x39;"
		"vperm2f128 ymm2, ymm3, ymm3, 0x81;"
		"vblendps ymm3, ymm3, ymm2, 0x88;"
	".endm;"
		"vzeroall;"
		"vmovups ymm4, [%[f]];"
		"vmovups ymm5, [%[f] + 32];"
		"vmovups ymm6, [%[f] + 64];"
		"vmovups ymm7, [%[f] + 96];"
	"_row:"
		"cmp %[h], 0;"
		"jle _end;"
		"dec %[h];"
		"xor %[ofs], %[ofs];"
		"vxorps ymm3, ymm3, ymm3;"
		"reaver 0;"
		"reaver 0;"
		"reaver 0;"
		"reaver 0;"
		"reaver 0;"
		"reaver 0;"
		"reaver 0;"
	"_col:"
		"reaver 1;"
		"add %[y], 4;"
		"cmp %[ofs], %[w];"
		"jl _col;"
		"jmp _row;"
	"_end:"
		"vzeroupper;"
		:
		: [ofs] "r" (0), [x] "r" (x), [y] "r" (y), [w] "r" (4*w), [h] "r" (h - 3), [f] "r" (vf)
		: "ymm0", "ymm1", "ymm2", "ymm3", "ymm4", "ymm5", "ymm6", "ymm7", "memory"
	);
}
#ifdef __cplusplus
}
#endif
