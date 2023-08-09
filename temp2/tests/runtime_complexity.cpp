#include "gtest/gtest.h"
#include "DistanceChecker.hpp"
#include <iostream>

namespace {



  TEST(RuntimeComplexity, WithAABB) {
	// Generate two polylines of given number (N,M) of points with greater threshold than 1.5
	// the AABB test will block the distance computation
	// Assuming complexity O((N-1)*(M-1))
	
	int N = 5;
	std::vector<sPoint2D> l1;
	for (int n = 0; n < N; n++) {
	  if (n % 2 == 0)
		l1.push_back({ 0,0 });
	  else
		l1.push_back({ 1,1 });
	}

	int computational_time[4];
	for (int j = 1; j < 5; j++) {
	  // number of points of the polylines
	  int M = 2 * pow(j, 3); // M = 2,16,54,128
	  std::vector<sPoint2D> l2;
	  for (int m = 0; m < M; m++) {
		if (m % 2 == 0)
		  l2.push_back({ 5,5 });
		else
		  l2.push_back({ 6,6 });
	  }

	  auto start = std::chrono::steady_clock::now();
	  for (int a = 0; a < 1e6; a++) {
		DistanceChecker checker(l1, l2, 1.5);
		checker.Check();
	  }
	  computational_time[j - 1] = std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now() - start).count();
	}
	std::cout << computational_time[0] << " " << computational_time[1] << " " << computational_time[2] << " " << computational_time[3] << std::endl;
  }
  

  TEST(RuntimeComplexity, NaiveApproach) {
	
  }
}  // namespace
