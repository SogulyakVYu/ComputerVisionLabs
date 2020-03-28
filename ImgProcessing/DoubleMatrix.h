#pragma once
#include <vector>

class DoubleMatrix
{
public:
	// ��� ���������� ������ �����������
	enum class BorderType
	{
		// ������ ����
		Black,
		// ��������� �������
		BorderPixel,
		// ��������� ����
		Reflect,
		// ������������� ����
		Wrap
	};
private:

	// ��� ���������� ������� ��� ������ ������ get
	static BorderType DefaultBorderType;
	// ������� ���� [1, 0, -1] ��� ���������� �����������
	static DoubleMatrix row101;
	// ������� ���� [1, 2 1] ��� ���������� ����������� (�������� ������)
	static DoubleMatrix sobelRow;
	// �������� ������ �� ���������� ������� ���������� ��� ���� �������
	std::vector<double> matrix;
	// ������ �������
	int height;
	// ������ �������
	int width;

	// ���������� ������� �� ������� (i, j) ����������� ��� ������ ���� �� ��������� �����������
	double getWithBlackBorder(int i, int j) const;
	// ���������� ������� �� ������� (i, j) ����������� ��� ��������� ������� �� ��������� �����������
	double getWithBorderPixel(int i, int j) const;
	// ���������� ������� �� ������� (i, j) ����������� ��� ��������� ����������� �� ��������
	double getWithReflectBorder(int i, int j) const;
	// ���������� ������� �� ������� (i, j) ����������� ��� "����������" ���� ����������� 
	double getWithWrapBorder(int i, int j) const;

	// ���������� ������� �� ������� (i, j)
	double get_(int i, int j) const { return matrix[i * width + j]; }

	// ���������� ������ (������) ���� ������� ������ �� ������� ����������=3*sigma 
	static int getGaussianSize(double sigma);
public:
	DoubleMatrix(int w, int h);
	DoubleMatrix(const DoubleMatrix& other);
	DoubleMatrix(const DoubleMatrix&& other);
	DoubleMatrix(std::vector<std::vector<double>> m);
	DoubleMatrix(std::initializer_list<std::initializer_list<double>> arr);

	int getWidth() const { return width; }
	int getHeight() const { return height; }

	DoubleMatrix& fillMatrix(double val);
	void set(int i, int j, double val);
	void set(int i, double val);
	// ���������� ������� �� ������� (i, j) ��� ������� ������� �� �������� �����������
	double get(int i, int j) const;

	// ������� �� ������
	DoubleMatrix convolutionRow(const DoubleMatrix& other) const;
	// ������� �� ������� (���� �������� � ���� ������)
	DoubleMatrix convolutionCol(const DoubleMatrix& other) const;
	// ������� �� �������������� ����
	DoubleMatrix convolution(const DoubleMatrix& other) const;
	// ������������ �������
	DoubleMatrix& normalize(double newMin, double newMax);
	// ���������� ����� ����������������� �������
	DoubleMatrix transpose();
	// ���������� ��������� ���������� ��������� ������  
	DoubleMatrix calcSobel();
	// ���������� ��������� ���������� ������� ������
	DoubleMatrix gaussian(double sigma);
	// ���������� ����������� �� X
	DoubleMatrix dx();
	// ���������� ����������� �� Y
	DoubleMatrix dy();
	DoubleMatrix add(double val);
	DoubleMatrix add(const DoubleMatrix& val);
	DoubleMatrix sub(double val);
	DoubleMatrix sub(const DoubleMatrix& val);
	DoubleMatrix mul(double val);
	DoubleMatrix mul(const DoubleMatrix& val);
	void printMatrix() const;

	// ��������� ���� ���������� ������ �����������
	static void setDefaultBoderType(BorderType type);
	// �������� ����������� � ����������� ������
	static void copyWithBorder(const DoubleMatrix& src, DoubleMatrix* dest, int xOffset, int yOffset);
	// ������� �� �������������� ����
	static DoubleMatrix convolution(const DoubleMatrix& f, const DoubleMatrix& h);
	static DoubleMatrix createGaussian(int width, int height, double sigma);
	static DoubleMatrix createGaussian(double sigma);
	// ������� ���� ������� ������ � ���� ������ �������� ������
	static DoubleMatrix createGaussianRow(int width, double sigma);
	static DoubleMatrix createGaussianRow(double sigma);
};

