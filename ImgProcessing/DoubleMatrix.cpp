#include <iostream>
#include <algorithm>
#include <numeric>
#include <functional>
#include <QtCore\qmath.h>
#include "DoubleMatrix.h"

DoubleMatrix::BorderType DoubleMatrix::DefaultBorderType = DoubleMatrix::BorderType::Reflect;
DoubleMatrix DoubleMatrix::row101 = DoubleMatrix{ {1, 0, -1} };
DoubleMatrix DoubleMatrix::sobelRow = DoubleMatrix{ {1, 2, 1} };

double DoubleMatrix::getWithBlackBorder(int i, int j) const
{
	if (i > -1 && j > -1 && i < height && j < width) return matrix[i * width + j];
	else return 0;
}

double DoubleMatrix::getWithBorderPixel(int i, int j) const
{
	int newI = i;
	int newJ = j;
	if (i < 0) newI = 0;
	else if (i >= height) newI = height - 1;
	
	if (j < 0) newJ = 0;
	else if (j >= width) newJ = width - 1;

	return matrix[newI * width + newJ];
}

double DoubleMatrix::getWithReflectBorder(int i, int j) const
{
	int newI = i;
	int newJ = j;

	if (i < 0) newI = -i - 1;
	else if (i >= height) newI = height - i % height - 1;
	else if (i >= height) newI = 0;

	if (j < 0) newJ = -j - 1;
	else if (j >= width) newJ = width - j % width - 1;
	else if (j >= width) newJ = 0;

	return matrix[(newI % height) * width + (newJ % width)];
}

double DoubleMatrix::getWithWrapBorder(int i, int j) const
{
	int newI = i;
	int newJ = j;

	if (i < 0) newI = (height - 1) + i + height;
	else if (i >= height) newI = i % height;

	if (j < 0) newJ = (width - 1) + j + width;
	else if (j >= width) newJ = j % width;

	return matrix[(newI % height) * width + (newJ % width)];
}

void DoubleMatrix::copyWithBorder(const DoubleMatrix& src, DoubleMatrix* dest, int wOffset, int hOffset)
{
	int newWidth = src.width + wOffset * 2;
	int newHeight = src.height + hOffset * 2;
	dest->matrix.resize(newWidth * newHeight);
	dest->height = newHeight;
	dest->width = newWidth;
	for (int i = 0; i < dest->height; i++) {
		for (int j = 0; j < dest->width; j++) {
			dest->matrix[i * dest->width + j] = src.get(i - hOffset, j - wOffset);
		}
	}
}

DoubleMatrix DoubleMatrix::convolutionRow(const DoubleMatrix& other) const
{
	int offsetH = 0;
	int offsetW = other.width / 2;
	DoubleMatrix result(this->width, this->height);

	DoubleMatrix* tmp = new DoubleMatrix(this->width, this->height);
	copyWithBorder(*this, tmp, offsetW, offsetH);

	for (int i = offsetH; i < result.height + offsetH; i++) {
		for (int j = offsetW; j < result.width + offsetW; j++) {
			double sum = 0;
			for (int v = -offsetW; v <= offsetW; v++) {
				sum += tmp->at(i, j - v) * other.at(0, v + offsetW);
			}
			result.set(i - offsetH, j - offsetW, sum);
		}
	}

	delete tmp;

	return result;
}

DoubleMatrix DoubleMatrix::convolutionCol(const DoubleMatrix& other) const
{
	int offsetH = other.width / 2;
	int offsetW = 0;
	DoubleMatrix result(this->width, this->height);

	DoubleMatrix* tmp = new DoubleMatrix(this->width, this->height);
	copyWithBorder(*this, tmp, offsetW, offsetH);

	for (int i = offsetH; i < result.height + offsetH; i++) {
		for (int j = offsetW; j < result.width + offsetW; j++) {
			double sum = 0;
			for (int v = -offsetH; v <= offsetH; v++) {
				sum += tmp->at(i - v, j) * other.at(0, v + offsetH);
			}
			result.set(i - offsetH, j - offsetW, sum);
		}
	}

	delete tmp;
	return result;
}

DoubleMatrix DoubleMatrix::convolution(const DoubleMatrix& other) const
{
	int offsetW = other.width / 2;
	int offsetH = other.height / 2;
	DoubleMatrix result(this->width, this->height);

	DoubleMatrix* tmp = new DoubleMatrix(this->width, this->height);
	copyWithBorder(*this, tmp, offsetW, offsetH);

	for (int i = offsetH; i < result.height + offsetH; i++) {
		for (int j = offsetW; j < result.width + offsetW; j++) {
			double sum = 0;

			for (int u = -offsetH; u <= offsetH; u++) {
				for (int v = -offsetW; v <= offsetW; v++) {
					int curY = i - u;
					int curX = j - v;
					sum += tmp->at(curY, curX) * other.at(u + offsetH, v + offsetW);
				}
			}

			result.set(i - offsetH, j - offsetW, sum);
		}
	}

	delete tmp;
	return result;
}

int DoubleMatrix::getGaussianSize(double sigma)
{
	int size = sigma * 3 * 2;
	if (size % 2 == 0) size += 1;
	return size;
}

DoubleMatrix::DoubleMatrix(): width(0), height(0), matrix(0) {}

DoubleMatrix::DoubleMatrix(int w, int h): width(w), height(h)
{
	matrix.resize(width * height);
}

DoubleMatrix::DoubleMatrix(const DoubleMatrix& other): width(other.width), height(other.height)
{
	matrix.resize(width * height);
	
	for (int i = 0; i < width * height; i++) {
		matrix[i] = other.matrix[i];
	}
}

DoubleMatrix::DoubleMatrix(std::vector<std::vector<double>> m)
{
	height = m.size();
	width = m[0].size();
	matrix.resize(height * width);

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			matrix[i * width + j] = m[i][j];
		}
	}
}

DoubleMatrix::DoubleMatrix(std::initializer_list<std::initializer_list<double>> arr)
{
	height = arr.size();
	width = arr.begin()->size();
	matrix.resize(width * height);

	int i = 0;
	for (auto& row : arr) {
		for (auto col : row) {
			matrix[i] = col;
			i++;
		}
	}
}

DoubleMatrix& DoubleMatrix::operator=(const DoubleMatrix& right)
{
	if (this == &right) return *this;
	width = right.width;
	height = right.height;
	std::copy(begin(right.matrix), end(right.matrix), begin(this->matrix));
	return *this;
}

double& DoubleMatrix::operator[](int i)
{
	return matrix[i];
}

DoubleMatrix& DoubleMatrix::fillMatrix(double val)
{
	fill(begin(matrix), end(matrix), val);
	return *this;
}

void DoubleMatrix::set(int i, int j, double val)
{
	matrix[i * width + j] = val;
}

void DoubleMatrix::set(int i, double val)
{
	matrix[i] = val;
}

double DoubleMatrix::get(int i, int j) const
{
	if (DefaultBorderType == BorderType::Black) return getWithBlackBorder(i, j);
	else if (DefaultBorderType == BorderType::BorderPixel) return getWithBorderPixel(i, j);
	else if (DefaultBorderType == BorderType::Reflect) return getWithReflectBorder(i, j);
	else if (DefaultBorderType == BorderType::Wrap) return getWithWrapBorder(i, j);
	else return -1;
}

DoubleMatrix& DoubleMatrix::normalize(double newMin, double newMax)
{
	//DoubleMatrix* result = new DoubleMatrix(*this);
	auto minMax = std::minmax_element(begin(matrix), end(matrix));
	double minEl = *minMax.first;
	double maxEl = *minMax.second;

	for (int i = 0; i < width * height; i++) {
		matrix[i] = (matrix[i] - minEl) * (newMax - newMin) / (maxEl - minEl) + newMin;
	}

	return *this;
}

DoubleMatrix DoubleMatrix::transpose()
{
	DoubleMatrix result(*this);
	result.height = width;
	result.width = height;
	if (height != 1 && width != 1) {

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				result.set(j, i, matrix[i * width + j]);
			}
		}
	}

	return result;
}

DoubleMatrix DoubleMatrix::calcSobel() const
{
	DoubleMatrix gX = dx();
	DoubleMatrix gY = dy();


	DoubleMatrix result(width, height);
	std::transform(begin(gX.matrix), end(gX.matrix), begin(gY.matrix), begin(result.matrix), 
		[](double x, double y) { return sqrt(x * x + y * y);});

	return result;
}

DoubleMatrix DoubleMatrix::gaussian(double sigma) const
{
	DoubleMatrix gaussianX = createGaussianRow(sigma);
	return this->convolutionRow(gaussianX).convolutionCol(gaussianX);
}

DoubleMatrix DoubleMatrix::dx() const
{
	return this->convolutionRow(row101).convolutionCol(sobelRow);
}

DoubleMatrix DoubleMatrix::dy() const
{
	return this->convolutionRow(sobelRow).convolutionCol(row101);
}

DoubleMatrix DoubleMatrix::add(double val) const
{
	DoubleMatrix result(width, height);
	std::transform(begin(matrix), end(matrix), begin(result.matrix), [&](double x) { return x + val; });
	return result;
}

DoubleMatrix DoubleMatrix::add(const DoubleMatrix& other) const
{
	DoubleMatrix result(width, height);
	std::transform(begin(matrix), end(matrix), begin(other.matrix), begin(result.matrix), std::plus<double>());
	return result;
}

DoubleMatrix DoubleMatrix::sub(double val) const
{
	DoubleMatrix result(width, height);
	std::transform(begin(matrix), end(matrix), begin(result.matrix), [&](double x) { return x - val; });
	return result;
}

DoubleMatrix DoubleMatrix::sub(const DoubleMatrix& other) const
{
	DoubleMatrix result(width, height);
	std::transform(begin(matrix), end(matrix), begin(other.matrix), begin(result.matrix), std::minus<double>());
	return result;
}

DoubleMatrix DoubleMatrix::mul(double val) const
{
	DoubleMatrix result(width, height);
	std::transform(begin(matrix), end(matrix), begin(result.matrix), [&](double x) { return x * val; });
	return result;
}

DoubleMatrix DoubleMatrix::mul(const DoubleMatrix& other) const
{
	DoubleMatrix result(width, height);
	std::transform(begin(matrix), end(matrix), begin(other.matrix), begin(result.matrix), std::multiplies<double>());
	return result;
}

DoubleMatrix DoubleMatrix::div(const DoubleMatrix& other) const
{
	DoubleMatrix result(width, height);
	std::transform(begin(matrix), end(matrix), begin(other.matrix), begin(result.matrix), std::divides<double>());
	return result;
}

DoubleMatrix DoubleMatrix::div(double val) const
{
	DoubleMatrix result(width, height);
	std::transform(begin(matrix), end(matrix), begin(result.matrix), [&](double x) { return x / val; });
	return result;
}

double DoubleMatrix::sum() const 
{
	return std::accumulate(begin(matrix), end(matrix), 0.0);
}

DoubleMatrix DoubleMatrix::abs() const
{
	DoubleMatrix result(width, height);
	std::transform(begin(matrix), end(matrix), begin(result.matrix), [&](double x) { return std::abs(x); });
	return result;
}

bool DoubleMatrix::allClose(DoubleMatrix& other, double eps)
{
	DoubleMatrix subMatrix = this->sub(other);
	auto minMaxIterator = std::minmax_element(begin(subMatrix.matrix), end(subMatrix.matrix));
	double absMax = std::max(std::abs(*minMaxIterator.first), std::abs(*minMaxIterator.second));
	//std::cout << "MAX=" << absMax << std::endl;
	return absMax <= eps;
}

DoubleMatrix DoubleMatrix::downsample(int pow)
{
	int k = std::pow(2, pow);
	DoubleMatrix result(this->width / k, this->height / k);
	Q_ASSERT(result.width != 0 && result.height != 0);

	for (int i = 0; i < result.height; i++) {
		for (int j = 0; j < result.width; j++) {
			result.matrix[i * result.width + j] = this->at(i * k, j * k);;
		}
	}

	return result;
}

void DoubleMatrix::printMatrix() const
{
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			std::cout << matrix[i * width + j] << " ";
		}
		std::cout << std::endl;
	}
}

void DoubleMatrix::setDefaultBoderType(BorderType type)
{
	DefaultBorderType = type;
}

DoubleMatrix DoubleMatrix::convolution(const DoubleMatrix& a, const DoubleMatrix& b)
{
	return a.convolution(b);
}

DoubleMatrix DoubleMatrix::createGaussian(int width, int height, double sigma)
{
	DoubleMatrix kernel(width, height);
	int offsetH = height / 2;
	int offsetW = width / 2;
	double sum = 0.0;

	for (int i = -offsetH; i <= offsetH; i++) {
		for (int j = -offsetW; j <= offsetW; j++) {
			double val = exp(-(i * i + j * j) / (2 * sigma * sigma)) / (2 * M_PI * sigma * sigma);
			kernel.set(i + offsetH, j + offsetW,  val);
			kernel.matrix[(i + offsetH) * width + (j + offsetW)] =  val;
			sum += val;
		}
	}

	std::transform(begin(kernel.matrix), end(kernel.matrix), begin(kernel.matrix), [&sum](double x) {return x / sum;});

	return kernel;
}

DoubleMatrix DoubleMatrix::createGaussian(double sigma)
{
	int size = getGaussianSize(sigma);
	return createGaussian(size, size, sigma);

}

DoubleMatrix DoubleMatrix::createGaussianRow(int width, double sigma)
{
	DoubleMatrix kernel(width, 1);
	int offsetW = width / 2;
	double sum = 0.0;

	for (int i = -offsetW; i <= offsetW; i++) {
		double val = exp(-(i * i) / (2 * sigma * sigma)) / (std::sqrt(2 * M_PI) * sigma);
		kernel.set(i + offsetW, val);
		kernel.matrix[i + offsetW] = val;
		sum += val;
	}

	for (int i = 0; i < width; i++) {
			kernel.matrix[i] /= sum;
	}

	return kernel;
}

DoubleMatrix DoubleMatrix::createGaussianRow(double sigma)
{
	return createGaussianRow(getGaussianSize(sigma), sigma);
}

DoubleMatrix operator+(const DoubleMatrix& a, const DoubleMatrix& b)
{
	return a.add(b);
}

DoubleMatrix operator+(const DoubleMatrix& a, double b)
{
	return a.add(b);
}

DoubleMatrix operator+(double a, const DoubleMatrix& b)
{
	return b.add(b);
}

DoubleMatrix operator-(const DoubleMatrix& a, const DoubleMatrix& b)
{
	return a.sub(b);
}

DoubleMatrix operator-(const DoubleMatrix& a, double b)
{
	return a.sub(b);
}

DoubleMatrix operator*(const DoubleMatrix& a, const DoubleMatrix& b)
{
	return a.mul(b);
}

DoubleMatrix operator*(const DoubleMatrix& a, double b)
{
	return a.mul(b);
}

DoubleMatrix operator*(double a, const DoubleMatrix& b)
{
	return b.mul(a);
}

DoubleMatrix operator/(const DoubleMatrix& a, const DoubleMatrix& b)
{
	return a.div(b);
}

DoubleMatrix operator/(const DoubleMatrix& a, double b)
{
	return a.div(b);
}

DoubleMatrix DoubleMatrix::gradientDirection() const
{
	DoubleMatrix dx = this->dx();
	DoubleMatrix dy = this->dy();
	DoubleMatrix result(width, height);
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			double rad = std::atan2(-dy.at(i,j), -dx.at(i, j));
			rad = rad + M_PI;
			result.set(i, j, rad);
		}
	}

	return result;
}