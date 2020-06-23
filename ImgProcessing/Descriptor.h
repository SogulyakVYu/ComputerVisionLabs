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
	int gridSize;
	double cellSize;
	int cellCount;
public:
	Descriptor(): values(), gridSize(0), cellSize(0), cellCount(0) {}
	// Создает дескриптор из заданного числа гистограмм и корзин в каждой гистограмме 
	Descriptor(int histogramCount, int binCount): values(binCount, histogramCount), gridSize(0), cellSize(0), cellCount(0)  {}
	// Создает дескриптор из заданного размера квадратной сетки (в пикселях), числа ячеек в сетке и числа корзин в каждой гистограмме
	Descriptor(int gridSize, int cellCount, int binCount): 
		values(binCount, cellCount * cellCount), gridSize(gridSize), cellCount(cellCount), cellSize((double)gridSize / cellCount) {}
	Descriptor(const DoubleMatrix& m): values(m), gridSize(0), cellSize(0), cellCount(0) {}
	Descriptor(const Descriptor& other): values(other.values), gridSize(other.gridSize), cellSize(other.cellSize), cellCount(other.cellCount) {}
	Descriptor(Descriptor&& other) = default;
	
	int getHistogramCount() const { return values.getHeight(); }
	int getBinCount() const { return values.getWidth(); }
	int getSize() const { return values.getSize(); }
	int getGridSize() const { return gridSize; }
	double getCellSize() const { return cellSize; }
	int getCellCount() const { return cellCount; }
	
	double& operator[](int i) { return values[i]; }

	double& at(int histogram, int bin) { return values[histogram * getBinCount() + bin]; }
	const double& at(int histogram, int bin) const { return values.at(histogram, bin); }
	DoubleMatrix& vals() { return values; }

	void set(int h, int b, double val);
	double length() const;
	void normalize();
	// Обрезать большие значения до максимума
	void truncate(double max);
	
	static double distance(Descriptor a, Descriptor b, DistanceType t = DistanceType::Default);
};

