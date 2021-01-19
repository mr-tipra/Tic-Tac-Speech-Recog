#ifndef UTIL_H
#define UTIL_H
#include <vector>
#include <exception>
#include <iostream>

template<typename T>
class Mat{
private:
	std::vector<std::vector<T>> data;
	int n, m;
public:
	
	Mat(int n, int m, T def = T()):data(n, std::vector<T>(m,def)), n(n), m(m){}
	
	std::vector<T>& operator[](int i){
		if(i < 0 || i >= n){
			std::cerr << "INVALID MATRIX INDEX\n";
			throw std::exception("Invalid Matrix Index");
		}
		return data[i];
	}
	int rows(){
		return n;
	}
	int cols(){
		int m;
	}
	//divide by num
	void divide(T val){
		for(int i=0; i<n; i++){
			for(int j=0; j<m; j++){
				data[i][j] /= val;
			}
		}
	}

	//sum
	Mat<T> operator+(const Mat<T>& other){
		if(n != other.n || m != other.m){
			std::cerr << "INVALID MATRIX ADDITION\n";
			throw std::exception("Invalid Matrix Addition");
		}
		Mat<T> newMat(n, m);
		for(int i=0; i<n; i++){
			for(int j=0; j<m; j++)
				newMat.data[i][j] = data[i][j] + other.data[i][j];
			
		}
		return newMat;
	}

	void print(){
		for(int i=0; i<n; i++){
			for(int j=0; j<m; j++)
				std::cout << data[i][j] << " ";
			std::cout << "\n";
		}
		std::cout << "\n";
	}

};

void logError(const std::string& error) {
	std::cerr << error << "\n";
	throw std::exception(error.c_str());
}
#endif