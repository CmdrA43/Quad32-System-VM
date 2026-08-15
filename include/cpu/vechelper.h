#ifndef VECHELPER_H
#define VECHELPER_H

#if defined(__AVX2__)
#include <immintrin.h>
struct vec8x32 {
	__m256i v;
	static vec8x32 load(const uint32_t* p){
		return { _mm256_loadu_si256((const __m256i*)p) };
	}
	
	void store(uint32_t* p) const {
		_mm256_storeu_si256((__m256i*)p, v);
	}
	
	vec8x32 operator+(const vec8x32& o) const {
		return { _mm256_add_epi32(v, o.v) };
	}
	
	vec8x32 operator-(const vec8x32& o) const {
		return { _mm256_sub_epi32(v, o.v) };
	}
	
	vec8x32 operator*(const vec8x32& o) const {
		return { _mm256_mullo_epi32(v, o.v) };
	}
};
#elif defined(__aarch64__)
#include <arm_neon.h>
struct vec8x32 {
	uint32x4_t lo, hi;
	static vec8x32 load(const uint32_t* p){
		return { vld1q_u32(p), vld1q_u32(p+4) };
	}
	
	void store(uint32_t* p) const{
		vst1q_u32(p, lo);
		vst1q_u32(p+4, hi);
	}
	
	vec8x32 operator+(const vec8x32& o) const{
		return { vaddq_u32(lo, o.lo), vaddq_u32(hi, o.hi) };
	}
	
	vec8x32 operator-(const vec8x32& o) const{
		return { vsubq_u32(lo, o.lo), vsubq_u32(hi, o.hi) };
	}
	
	vec8x32 operator*(const vec8x32& o) const{
		return { vmulq_u32(lo, o.lo), vmulq_u32(hi, o.hi) };
	}
};

#else
struct vec8x32 {
	uint32_t data[8];
	
	static vec8x32 load(const uint32_t* p){
		vec8x32 v;
		for(int i = 0; i < 8; ++i) v.data[i] = p[i];
		return v;
	}
	
	void store(uint32_t* p) const{
		for(int i = 0; i < 8; ++i) p[i] = v.data[i];
	}
	
	vec8x32 operator+(const vec8x32& o){
		vec8x32 v;
		for(int i = 0; i < 8; ++i) v[i] = data[i] + o.data[i];
		return v;
	}
	
	vec8x32 operator-(const vec8x32& o){
		vec8x32 v;
		for(int i = 0; i < 8; ++i) v[i] = data[i] - o.data[i];
		return v;
	}
	
	vec8x32 operator*(const vec8x32& o){
		vec8x32 v;
		for(int i = 0; i < 8; ++i) v[i] = data[i] * o.data[i];
		return v;
	}
};

#endif

#endif
