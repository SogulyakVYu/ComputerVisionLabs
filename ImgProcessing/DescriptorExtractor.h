#pragma once
#include <vector>
#include "DoubleMatrix.h"
#include "KeyPoint.h"
#include "Descriptor.h"
class DescriptorExtractor
{
private:
	// Размер квадратной сетки в пикселях
	int gridSize;
	// Размер одной ячейки (области) в пикселях
	int cellSize;
	// Число ячеек в сетке
	int cellCount;
	// Число гистограмм
	int histogramCount;
	// Число корзин в гистограмме
	int binCount;

	// Заполнение одного дескриптора
	void fillDescriptor(Descriptor& descriptor, const DoubleMatrix& dirs, const DoubleMatrix& grad, KeyPoint& point);
	// Возвращает индексы корзин для указанного угла
	static std::pair<int, int> getBinsIndexies(double phi, double binSize, int binCount);
public:
	// Инициалзиция из размера сетки, числа ячеек в сетке, числа гистограмм и числа корзин в одной гистограмме
	DescriptorExtractor(int gridSize, int cellCount, int histogramCount, int binCount);
	// Инициалзиция из размера сетки, числа ячеек в сетке, числа корзин в одной гистограмме (число гистограмм равно числу ячеек в квадрате)
	DescriptorExtractor(int gridSize, int cellCount, int binCount);
	// Вычисление дескрипторов изображения на основе заданных точек
	std::vector<Descriptor> compute(const DoubleMatrix& img, std::vector<KeyPoint>& points);
	// Поиск ближайших дескрипторов
	static std::vector<std::pair<int, int>> findMatches(std::vector<Descriptor> aDescriptors, std::vector<Descriptor> bDescriptors, double threshold = 0.66);
};

