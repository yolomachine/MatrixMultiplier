#pragma once
#define EPS 2e-3
#include <thread>
#include <functional>
#include <atomic>
#include <vector>
#include <utility>

size_t threads_number;

template<typename T>
class Matrix {
public:
	typedef std::pair<T, T> size;
	typedef std::vector<T> vector;
	typedef std::vector<vector> table;

	Matrix() {};
	Matrix(size_t n) : data(n, vector(n, T())), _size(n, n) {};
	Matrix(size_t n, T val) : data(n, vector(n, val)), _size(n, n) {};
	Matrix(size s) : data(s.first, vector(s.second, T())), _size(s) {};
	Matrix(size s, T val) : data(s.first, vector(s.second, val)), _size(s) {};
	Matrix(size_t n, size_t m, T val) : data(n, vector(m, val)), _size(n, m) {};

	std::vector<T>& operator[](size_t i) { return data[i]; };
	Matrix operator*(Matrix);
	bool operator==(Matrix);

	void thread_matrix_partition_multiply(size_t thread_num, size_t n, size_t m, Matrix B, Matrix& res);
	Matrix<T> Matrix<T>::no_thread_matrix_partition_multiply(Matrix<T> B);

private:
	table data;
	size _size;
};

template<typename T>
Matrix<T> Matrix<T>::no_thread_matrix_partition_multiply(Matrix<T> other) {
	Matrix<T> res(this->_size.first, other._size.second, 0);
	for (size_t i = 0; i < this->_size.first; ++i)
		for (size_t j = 0; j < other._size.second; ++j)
			for (size_t k = 0; k < other._size.second; ++k)
				res.data[i][j] += this->data[i][k] * other.data[k][j];
	return res;
}

template<typename T>
void Matrix<T>::thread_matrix_partition_multiply(size_t thread_num, size_t n, size_t m, Matrix<T> B, Matrix<T>& res) {
	const size_t begin = (thread_num * n) / threads_number;
	const size_t end = ((thread_num + 1) * n) / threads_number;
	for (size_t i = begin; i < end; ++i)
		for (size_t j = 0; j < m; ++j)
			for (size_t k = 0; k < m; ++k)
				res.data[i][j] += this->data[i][k] * B.data[k][j];
};

template<typename T>
Matrix<T> Matrix<T>::operator*(Matrix<T> other) {
	Matrix<T> res(this->_size.first, other._size.second, 0);
	std::vector<std::thread> threads;
	for (size_t i = 0; i < threads_number; ++i)
		threads.push_back(std::thread(&Matrix<T>::thread_matrix_partition_multiply, this, i, res._size.first, res._size.second, other, std::ref(res)));
	for (size_t i = 0; i < threads.size(); ++i)
		threads[i].join();
	return res;
}

template<typename T>
bool Matrix<T>::operator==(Matrix<T> other) {
	if (this->data.size() != other.data.size() || this->data[0].size() != other.data[0].size())
		return false;

	for (size_t i = 0; i < this->data.size(); ++i)
		for (size_t j = 0; j < this->data[0].size(); ++j)
			if (!(abs(this->data[i][j] - other.data[i][j]) < EPS)) return false;

	return true;
}