#pragma once
#include "DoubleMatrix.h"
#include "KeyPoint.h"
class Descriptor
{
public:
	// Расстояние между дескрипторами
	enum class DistanceType
	{
		// Метрика Маннхэттена
		L1,
		// Эвклидово расстояние
		L2,
		// Sum of squared distances
		SSD,
		Default = L2
	};
private:
	DoubleMatrix values;
public:
	Descriptor(): values() {}
	// Создает дескриптор из заданного числа гистограмм и корзин в каждой гистограмме 
	Descriptor(int histogramCount, int binCount): values(binCount, histogramCount) {}
	// Создает дескриптор из заданного размера квадратной сетки (в пикселях), числа ячеек в сетке и числа корзин в каждой гистограмме
	Descriptor(int gridSize, int cellCount, int binCount): values(binCount, cellCount * cellCount) {}
	Descriptor(const DoubleMatrix& m): values(m) {}
	Descriptor(const Descriptor& other): values(other.values) {}
	Descriptor(Descriptor&& other) = default;
	
	int getHistogramCount() const { return values.getHeight(); }
	int getBinCount() const { return values.getWidth(); }
	int getSize() const { return values.getSize(); }
	
	double& operator[](int i) { return values[i]; }

	double& at(int histogram, int bin) { return values[histogram * getBinCount() + bin]; }
	const double& at(int histogram, int bin) const { return values.at(histogram, bin); }
	const DoubleMatrix& vals(){ return values; }

	void set(int h, int b, double val);
	double length() const;
	void normalize();
	// Обрезать большие значения до максимума
	void truncate(double max);
	
	static double distance(Descriptor a, Descriptor b, DistanceType t = DistanceType::Default);
};

