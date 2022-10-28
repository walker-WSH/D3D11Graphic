#include "pch.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "convert-video-color.h"

struct vec4 {
	union {
		struct {
			float x, y, z, w;
		};
		float ptr[4];
		__m128 m;
	};
};

struct matrix4 {
	struct vec4 x, y, z, t;
};

static inline void get_3x3_submatrix(float *dst, const struct matrix4 *m, int i, int j)
{
	const float *mf = (const float *)m;
	int ti, tj, idst, jdst;

	for (ti = 0; ti < 4; ti++) {
		if (ti < i)
			idst = ti;
		else if (ti > i)
			idst = ti - 1;
		else
			continue;

		for (tj = 0; tj < 4; tj++) {
			if (tj < j)
				jdst = tj;
			else if (tj > j)
				jdst = tj - 1;
			else
				continue;

			dst[(idst * 3) + jdst] = mf[(ti * 4) + tj];
		}
	}
}

static inline float get_3x3_determinant(const float *m)
{
	return (m[0] * ((m[4] * m[8]) - (m[7] * m[5]))) - (m[1] * ((m[3] * m[8]) - (m[6] * m[5]))) +
	       (m[2] * ((m[3] * m[7]) - (m[6] * m[4])));
}

float matrix4_determinant(const struct matrix4 *m)
{
	const float *mf = (const float *)m;
	float det, result = 0.0f, i = 1.0f;
	float m3x3[9];
	int n;

	for (n = 0; n < 4; n++, i = -i) { // NOLINT(clang-tidy-cert-flp30-c)
		get_3x3_submatrix(m3x3, m, 0, n);

		det = get_3x3_determinant(m3x3);
		result += mf[n] * det * i;
	}

	return result;
}

bool matrix4_inv(struct matrix4 *dst, const struct matrix4 *m)
{
	struct vec4 *dstv;
	float det;
	float m3x3[9];
	int i, j, sign;

	if (dst == m) {
		struct matrix4 temp = *m;
		return matrix4_inv(dst, &temp);
	}

	dstv = (struct vec4 *)dst;
	det = matrix4_determinant(m);

	if (fabs(det) < 0.0005f)
		return false;

	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			sign = 1 - ((i + j) % 2) * 2;
			get_3x3_submatrix(m3x3, m, i, j);
			dstv[j].ptr[i] = get_3x3_determinant(m3x3) * (float)sign / det;
		}
	}

	return true;
}

namespace matrix {
bool GetVideoMatrix(enum video_range_type color_range, enum video_colorspace color_space,
		    std::array<float, 16> *color_matrix, std::array<float, 3> *color_range_min,
		    std::array<float, 3> *color_range_max)
{
	for (const auto &info : format_info) {
		if (info.color_space != color_space)
			continue;

		if (color_range == video_range_type::VIDEO_RANGE_FULL) {
			*color_matrix = info.matrix[1];

			if (color_range_min)
				*color_range_min = {0.0f, 0.0f, 0.0f};

			if (color_range_max)
				*color_range_max = {1.0f, 1.0f, 1.0f};
		} else {
			*color_matrix = info.matrix[0];

			if (color_range_min)
				*color_range_min = info.float_range_min;

			if (color_range_max)
				*color_range_max = info.float_range_max;
		}

		return true;
	}

	assert(false);
	return false;
}

void VideoMatrixINV(std::array<float, 16> &color_matrix)
{
	struct matrix4 mtx;

	mtx.x.ptr[0] = color_matrix[0];
	mtx.x.ptr[1] = color_matrix[1];
	mtx.x.ptr[2] = color_matrix[2];
	mtx.x.ptr[3] = color_matrix[3];

	mtx.y.ptr[0] = color_matrix[4];
	mtx.y.ptr[1] = color_matrix[5];
	mtx.y.ptr[2] = color_matrix[6];
	mtx.y.ptr[3] = color_matrix[7];

	mtx.z.ptr[0] = color_matrix[8];
	mtx.z.ptr[1] = color_matrix[9];
	mtx.z.ptr[2] = color_matrix[10];
	mtx.z.ptr[3] = color_matrix[11];

	mtx.t.ptr[0] = color_matrix[12];
	mtx.t.ptr[1] = color_matrix[13];
	mtx.t.ptr[2] = color_matrix[14];
	mtx.t.ptr[3] = color_matrix[15];

	matrix4_inv(&mtx, &mtx);

	struct vec4 r_row = mtx.x;
	mtx.x = mtx.y;
	mtx.y = r_row;

	color_matrix[0] = mtx.x.ptr[0];
	color_matrix[1] = mtx.x.ptr[1];
	color_matrix[2] = mtx.x.ptr[2];
	color_matrix[3] = mtx.x.ptr[3];

	color_matrix[4] = mtx.y.ptr[0];
	color_matrix[5] = mtx.y.ptr[1];
	color_matrix[6] = mtx.y.ptr[2];
	color_matrix[7] = mtx.y.ptr[3];

	color_matrix[8] = mtx.z.ptr[0];
	color_matrix[9] = mtx.z.ptr[1];
	color_matrix[10] = mtx.z.ptr[2];
	color_matrix[11] = mtx.z.ptr[3];

	color_matrix[12] = mtx.t.ptr[0];
	color_matrix[13] = mtx.t.ptr[1];
	color_matrix[14] = mtx.t.ptr[2];
	color_matrix[15] = mtx.t.ptr[3];
}
}
