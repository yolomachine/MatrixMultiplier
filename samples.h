#pragma once
#include <fstream>
#include "matrix.hpp"

namespace sample {

	Matrix<int> _no_thread_50x50() {
		Matrix<int> A(50, 10);
		Matrix<int> B(50, 20);
		return A.no_thread_matrix_partition_multiply(B);
	}

	Matrix<int> _no_thread_500x500() {
		Matrix<int> A(500, 10);
		Matrix<int> B(500, 20);
		return A.no_thread_matrix_partition_multiply(B);
	}

	Matrix<int> _50x50() {
		Matrix<int> A(50, 10);
		Matrix<int> B(50, 20);
		return A * B;
	}

	Matrix<int> _500x500() {
		Matrix<int> A(500, 10);
		Matrix<int> B(500, 20);
		return A * B;
	}

	Matrix<double> _hilb(int n) {
		Matrix<double> A(n, .0);
		Matrix<double> B(n, .0);
		for (size_t i = 0; i < n; ++i)
			for (size_t j = 0; j < n; ++j) {
				A[i][j] = 1.0 / (i + j + 1);
				B[i][j] = 1.0 / (i + j + 1);
			}
		return A * B;
	}

	void test() {
		Matrix<double> hilb_squared(10, .0);
		std::fstream fin("input_hilb_mx.txt");
		for (size_t i = 0; i < 10; ++i)
			for (size_t j = 0; j < 10; ++j)
				fin >> hilb_squared[i][j];
		fin.close();

		if (hilb_squared == _hilb(10)) std::cout << "Correct multiplication: hilb(10)" << std::endl;
		if (Matrix<int>(50, 50 * 200) == _50x50()) std::cout << "Correct multiplication: 50 x 50" << std::endl;
		if (Matrix<int>(500, 500 * 200) == _500x500()) std::cout << "Correct multiplication: 500 x 500" << std::endl;
	}

}