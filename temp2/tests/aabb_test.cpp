#include "gtest/gtest.h"
#include "DistanceChecker.hpp"
#include <iostream>
#include <chrono>

namespace {
  TEST(AabbTest, ComputTime) {
	// 2 line segment that does only need aabb-test
	std::vector<sPoint2D> l1 = { {0,0},{0,1} };
	std::vector<sPoint2D> l2 = { {5,0},{5,1} };

	DistanceChecker checker(l1, l2, 1.5);

	// Measure the computational time of the checking with aabb filtering
	auto start = std::chrono::steady_clock::now();
	int N = 1e6;
	for (int n = 0; n < N; n++)
	  checker.Check();
	int dt_aabb = std::chrono::duration_cast<std::chrono::milliseconds>(
	  std::chrono::steady_clock::now() - start).count();
	
	// Measure the computational time of the naive way
	start = std::chrono::steady_clock::now();
	for (int n = 0; n < N; n++)
	  checker.CheckNaive();
	int dt_naive = std::chrono::duration_cast<std::chrono::milliseconds>(
	  std::chrono::steady_clock::now() - start).count();

	//std::cout << dt_naive << " " << dt_aabb << std::endl;
	EXPECT_LE(dt_aabb, dt_naive*0.4); // in debug mode ~50%, in release mode ~20%
  }
  
  float myrand() {
	return float(rand()) / 1e4;
  }

  TEST(AabbTest, Accuracy) {
	srand((unsigned)time(NULL));
	int N = 1e5;
	for (int n = 0; n < N; n++) {
	  DistanceChecker checker({ sPoint2D(myrand(), myrand()), sPoint2D(myrand(), myrand()) },
		{ sPoint2D(myrand(), myrand()), sPoint2D(myrand(), myrand()) },1.5);
	  EXPECT_EQ(checker.Check(), checker.CheckNaive());
	}
  }
}  // namespace
