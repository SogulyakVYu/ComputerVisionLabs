#pragma once
#include <vector>
#include <functional>
#include "KeyPoint.h"

class DoubleMatrix
{
public:
	// Тип заполнения границ изображения
	enum class BorderType
	{
		// Черный цвет
		Black,
		// Граничный пиксель
		BorderPixel,
		// Отражение края
		Reflect,
		// Заворачивание края
		Wrap
	};
private:
	// Тип заполнения границы при вызове метода get
	static BorderType DefaultBorderType;
	// Матрица вида [1, 0, -1] для вычисления производных
	static DoubleMatrix row101;
	// Матрица вида [1, 2 1] для вычисления производных (Оператор Собеля)
	static DoubleMatrix sobelRow;
	// Основной вектор со значениями яркости избражения или ядра свертки
	std::vector<double> matrix;
	// Высота матрицы
	int height;
	// Ширина матрицы
	int width;

	// Возвращает пиксель на позиции (i, j) изображения или черный цвет за границами изображения
	double getWithBlackBorder(int i, int j) const;
	// Возвращает пиксель на позиции (i, j) изображения или граничный пиксель за границами изображения
	double getWithBorderPixel(int i, int j) const;
	// Возвращает пиксель на позиции (i, j) изображения или отражение изображения за границей
	double getWithReflectBorder(int i, int j) const;
	// Возвращает пиксель на позиции (i, j) изображения или "завернутый" край изображения 
	double getWithWrapBorder(int i, int j) const;

	// Ошибка при сдвиге окна в детекторе Моравека
	double moravecC(int x, int y, const std::vector<int>& windowSize, const std::vector<int>& d);

	// Возвращает размер (ширину) ядра фильтра гаусса по правилу полуразмер=3*sigma 
	static int getGaussianSize(double sigma);
	// Оригинальный оператор Харриса 
	static DoubleMatrix harrisF(DoubleMatrix& a, DoubleMatrix& b, DoubleMatrix& c, double coef = 0.04);
	// Оператор Харриса с использованием lambda min
	static DoubleMatrix harrisE(DoubleMatrix& a, DoubleMatrix& b, DoubleMatrix& c);
public:
	DoubleMatrix();
	DoubleMatrix(int w, int h);
	DoubleMatrix(const DoubleMatrix& other);
	DoubleMatrix(DoubleMatrix&& other) = default;
	DoubleMatrix(std::vector<std::vector<double>> m);
	DoubleMatrix(std::initializer_list<std::initializer_list<double>> arr);

	int getWidth() const { return width; }
	int getHeight() const { return height; }

	DoubleMatrix& operator=(const DoubleMatrix& right);
	DoubleMatrix& operator=(DoubleMatrix&& right) = default;
	double& operator[](int i);
	friend DoubleMatrix operator+(const DoubleMatrix& a, const DoubleMatrix& b);
	friend DoubleMatrix operator+(const DoubleMatrix& a, double b);
	friend DoubleMatrix operator+(double a, const DoubleMatrix& b);
	friend DoubleMatrix operator-(const DoubleMatrix& a, const DoubleMatrix& b);
	friend DoubleMatrix operator-(const DoubleMatrix& a, double b);
	friend DoubleMatrix operator*(const DoubleMatrix& a, const DoubleMatrix& b);
	friend DoubleMatrix operator*(const DoubleMatrix& a, double b);
	friend DoubleMatrix operator*(double a, const DoubleMatrix& b);
	friend DoubleMatrix operator/(const DoubleMatrix& a, const DoubleMatrix& b);
	friend DoubleMatrix operator/(const DoubleMatrix& a, double b);

	DoubleMatrix& fillMatrix(double val);
	void set(int i, int j, double val);
	void set(int i, double val);
	// Возвращает пиксель на позиции (i, j) или заданый пиксель за границей изображения
	double get(int i, int j) const;
	// Возвращает пиксель на позиции (i, j)
	double at(int i, int j) const { return matrix[i * width + j]; }
	double at(int i) const { return matrix[i]; }

	// Свертка по строке
	DoubleMatrix convolutionRow(const DoubleMatrix& other) const;
	// Свертка по столбцу (ядро задается в виде строки)
	DoubleMatrix convolutionCol(const DoubleMatrix& other) const;
	// Свертка по прямоугольному ядру
	DoubleMatrix convolution(const DoubleMatrix& other) const;
	// Нормирование матрицы
	DoubleMatrix& normalize(double newMin, double newMax);
	// Возвращает копию транспонированной матрицы
	DoubleMatrix transpose();
	// Возвращает результат применения оператора Собеля  
	DoubleMatrix calcSobel();
	// Возвращает результат применения фильтра Гаусса
	DoubleMatrix gaussian(double sigma);
	// Возвращает производную по X
	DoubleMatrix dx();
	// Возвращает производную по Y
	DoubleMatrix dy();
	DoubleMatrix add(double val) const;
	DoubleMatrix add(const DoubleMatrix& mat) const;
	DoubleMatrix sub(double val) const;
	DoubleMatrix sub(const DoubleMatrix& mat) const;
	DoubleMatrix mul(double val) const;
	DoubleMatrix mul(const DoubleMatrix& mat) const;
	DoubleMatrix div(const DoubleMatrix& mat) const;
	DoubleMatrix div(double val) const;
	bool allClose(DoubleMatrix& other, double eps);
	// Уменьшает размер изображения в два раза
	DoubleMatrix downsample(int pow = 1);
	void printMatrix() const;

	DoubleMatrix& norm1() { return normalize(0, 1); }
	DoubleMatrix& norm255() { return normalize(0, 255); }
	// Детектор углов Моравека
	DoubleMatrix operatorMoravec(int windowSize);
	// Детектор углов Харриса
	DoubleMatrix operatorHarris(int windowSize);
	// Возращает набор интересных точек соответствующих локальным максимумам выше заданного порога
	std::vector<KeyPoint> getLocalMax(const int windowSize, double threshold);
	std::vector<KeyPoint> getLocalMax(const std::vector<int>& windowSize, double threshold);
	// Возвращает набор интересных точек, значения которых выше порога
	std::vector<KeyPoint> getKeyPoints(double threshold);

	// Установка типа заполнения границ изображения
	static void setDefaultBoderType(BorderType type);
	// Копирует изображение с добавлением границ
	static void copyWithBorder(const DoubleMatrix& src, DoubleMatrix* dest, int xOffset, int yOffset);
	// Свертка по прямоугольному ядру
	static DoubleMatrix convolution(const DoubleMatrix& f, const DoubleMatrix& h);
	static DoubleMatrix createGaussian(int width, int height, double sigma);
	static DoubleMatrix createGaussian(double sigma);
	// Создает ядро фильтра Гаусса в виде строки заданной ширины
	static DoubleMatrix createGaussianRow(int width, double sigma);
	static DoubleMatrix createGaussianRow(double sigma);
};

