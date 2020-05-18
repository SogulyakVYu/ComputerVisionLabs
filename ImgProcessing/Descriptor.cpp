#include <QtCore/qdebug.h>
#include <QtCore/qmath.h>
#include "Descriptor.h"

void Descriptor::set(int h, int b, double val)
{
	values.set(h, b, val);
}

double Descriptor::length() const
{
	DoubleMatrix sqr = (values * values);
	return std::sqrt(sqr.sum());
}

void Descriptor::normalize()
{
	double len = length();
	values = values / len;
}

void Descriptor::truncate(double max)
{
	for (int i = 0; i < values.getSize(); i++) {
		if (values[i] > max) {
			values[i] = max;
		}
	}
}

double Descriptor::distance(Descriptor a, Descriptor b, DistanceType t)
{
	DoubleMatrix diff = a.values - b.values;
	if (t == DistanceType::L2) {
		DoubleMatrix square = diff * diff;
		return std::sqrt(square.sum());
	}
	else if (t == DistanceType::L1) {
		return diff.abs().sum();
	}
	else if (t == DistanceType::SSD) {
		DoubleMatrix square = diff * diff;
		return square.sum();
	}

	return -1;
}
