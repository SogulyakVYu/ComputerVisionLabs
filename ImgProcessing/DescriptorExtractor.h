#pragma once
#include <vector>
#include "DoubleMatrix.h"
#include "KeyPoint.h"
#include "Descriptor.h"
#include "Pyramid.h"
class DescriptorExtractor
{
private:
	// Размер квадратной сетки в пикселях
	int extractorGridSize;
	// Размер одной ячейки (области) в пикселях
	int extractorCellSize;
	// Число ячеек в сетке
	int extractorCellCount;
	// Число гистограмм
	int extractorHistogramCount;
	// Число корзин в гистограмме
	int extractorBinCount;

	// Заполнение одного дескриптора
	void fillDescriptor(Descriptor& descriptor, const DoubleMatrix& dirs, const DoubleMatrix& grad, KeyPoint& point);
	// Заполнение одного дескриптора с учетом угла точки
	void fillDescriptorAngle(Descriptor& descriptor, const DoubleMatrix& dirs, const DoubleMatrix& grad, KeyPoint& point);
	// Заполнение одного дескриптора угол и масштаб
	void fillDescriptorScale(Descriptor& descriptor, const DoubleMatrix& dirs, const DoubleMatrix& grad, KeyPoint& point);
	// Вычисленние гистограммы ориентации градиентов для точки
	static void calcOrientationHistogram(Descriptor& descriptor, const DoubleMatrix& dirs, const DoubleMatrix& grad, KeyPoint& point);
	// Добавляет одну или две точки с разной ориентацией в список на основе значения пиков гистограммы
	static void addPointWithPeaks(KeyPoint& point, Descriptor& descriptor, std::vector<KeyPoint>& out, int bins);
	// Возвращает индексы корзин для указанного угла
	static std::pair<int, int> getBinsIndexies(double phi, double binSize, int binCount);

	std::vector<KeyPoint> gridPoints;
public:
	// Инициалзиция из размера сетки, числа ячеек в сетке, числа гистограмм и числа корзин в одной гистограмме
	DescriptorExtractor(int gridSize, int cellCount, int histogramCount, int binCount);
	// Инициалзиция из размера сетки, числа ячеек в сетке, числа корзин в одной гистограмме (число гистограмм равно числу ячеек в квадрате)
	DescriptorExtractor(int gridSize, int cellCount, int binCount);
	// Вычисление дескрипторов изображения на основе заданных точек
	std::vector<Descriptor> compute(const DoubleMatrix& img, std::vector<KeyPoint>& points);
	// Вычисление дескрипторов изображения на основе заданных точек
	std::pair<std::vector<KeyPoint>, std::vector<Descriptor>> computeScale(Pyramid& pyramid, std::vector<KeyPoint>& points);
	// Определение угла интересной точки
	static std::vector<KeyPoint> calcPointsOrientation(const DoubleMatrix& img, std::vector<KeyPoint>& points, int bins = 36);
	// Поиск ближайших дескрипторов
	static std::vector<std::pair<int, int>> findMatches(std::vector<Descriptor> aDescriptors, std::vector<Descriptor> bDescriptors, double threshold = 0.66);

	std::vector<KeyPoint> getGridPoints() { return gridPoints; }
};

