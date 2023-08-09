#include "gtest/gtest.h"
#include "DistanceChecker.hpp"
#include <iostream>

namespace {
  void CheckDistance(const sPoint2D& p0, const sPoint2D& p1,
	const sPoint2D& q0, const sPoint2D& q1) {
	float s, t;
	float d = DistanceChecker::_getDistance(p0, p1, q0, q1, s, t);

	EXPECT_GE(s,0);
	EXPECT_LE(s,1);
	EXPECT_GE(t,0);
	EXPECT_LE(t,1);
	// Check the value of the distance of given points
	{
	  auto p = affine_combination(p0, p1, s);
	  auto q = affine_combination(q0, q1, t);
	  float d_ = (p - q).length();
	  if (d_ < 5e-3 && d < 5e-3)
		return; // Collision with zero distance
	  EXPECT_LE(abs(d - d_), d_ * (1.f + 1e-5) + 1e-5);
	}
	// Check if it is the distance of the segments
	float eps = 1e-2;
	for (int i = -1; i < 2; i++) {
	  float s_ = s + float(i) * eps;
	  if (s_ >= 0 && s_ <= 1)
		for (int j = -1; j < 2; j++) {
		  float t_ = t + float(j) * eps;
		  if (t_ >= 0 && t_ <= 1 && !(i==0 && j==0)) {
			auto p = affine_combination(p0, p1, s_);
			auto q = affine_combination(q0, q1, t_);
			float d_ = (p - q).length();
			EXPECT_GE(d_ * (1.f + 5e-5) + 5e-5, d);
		  }
		}
	}
  }
  
  TEST(DistanceOfSegments, Cornercases) {
	auto p0 = sPoint2D(0, 0);
	auto p1 = sPoint2D(1, 0);
	// orthogonal cases
	// t=0.5
	CheckDistance(p0, p1, { -1,-0.5 }, { -1,0.5 }); // s=0
	CheckDistance(p0, p1, { 0.5,-0.5 }, { 0.5,0.5 }); // s=0.5
	CheckDistance(p0, p1, { 1.5,-0.5 }, { 1.5,0.5 }); // s=1

	// t=0
	CheckDistance(p0, p1, { -1,-0.5 }, { -1,-1.5 }); // s=0
	CheckDistance(p0, p1, { 0.5,-0.5 }, { 0.5,-1.5 }); // s=0.5
	CheckDistance(p0, p1, { 1.5,-0.5 }, { 1.5,-1.5 }); // s=1

	// t=1
	CheckDistance(p0, p1, { -1,-2.5 }, { -1,-1.5 }); // s=0
	CheckDistance(p0, p1, { 0.5,-2.5 }, { 0.5,-1.5 }); // s=0.5
	CheckDistance(p0, p1, { 1.5,-2.5 }, { 1.5,-1.5 }); // s=1

	// parallel-colline cases
	CheckDistance(p0, p1, { 0,0 }, {1,0 });
	CheckDistance(p0, p1, { 0.5,0 }, { 1.5,0 });
	CheckDistance(p0, p1, { 1,0 }, { 1.5,0 });

	// parallel cases w/o common points
	CheckDistance(p0, p1, { 0,1 }, { 1,1 });
	CheckDistance(p0, p1, { 0.5,1 }, { 1.5,1 });
	CheckDistance(p0, p1, { 1,1 }, { 1.5,1 });
  }
  
  float myrand() {
	return float(rand()) / 1e4;;
  }

  TEST(DistanceOfSegments, Random) {
	srand((unsigned)time(NULL));
	int N = 1e6;
	for (int n = 0; n < N; n++) 
	  CheckDistance(sPoint2D(myrand(), myrand()), sPoint2D(myrand(), myrand()),
		sPoint2D(myrand(), myrand()), sPoint2D(myrand(), myrand()));
  }
}  // namespace
