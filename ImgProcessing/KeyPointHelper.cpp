#include <algorithm>
#include "KeyPointHelper.h"

std::vector<KeyPoint> KeyPointHelper::anms(std::vector<KeyPoint>& points, int pointsCount, double minR, double maxR)
{
	double r = minR;
	double step = 1;
	while (points.size() > pointsCount&& r < maxR)
	{
		auto toRemove = std::remove_if(begin(points), end(points),
			[&](KeyPoint& pointA) {
				for (auto& pointB : points) {
					if (KeyPoint::distance(pointA, pointB) < r && pointA.f < pointB.f) {
						return true;
					}
				}
				return false;
			});
		points.erase(toRemove, end(points));
		r += step;
	}

	return points;
}

std::vector<KeyPoint> KeyPointHelper::brownAnms(std::vector<KeyPoint>& points, int pointCount)
{
	std::vector<std::pair<float, int>> minDists;
	constexpr float maxFloat = std::numeric_limits<float>::max();
	minDists.push_back({ maxFloat, 0 });
	for (int i = 1; i < points.size(); i++) {
		float minDist = maxFloat;
		for (int j = 0; j < i; j++) {
			float curDist = KeyPoint::distance(points[j], points[i]);
			minDist = std::min(curDist, minDist);
		}
		minDists.push_back({ minDist, i });
	}

	std::sort(begin(minDists), end(minDists),
		[&](const std::pair<float, int>& a, const std::pair<float, int>& b) {
			return a.first > b.first;
		});
	std::vector<KeyPoint> result;
	for (int i = 0; i < pointCount; i++) result.push_back(points[minDists[i].second]);

	return result;
}

std::vector<KeyPoint> KeyPointHelper::getLocalMax(const DoubleMatrix& img, const int windowSize, double threshold)
{
	return getLocalMax(img, { windowSize, windowSize }, threshold);
}

std::vector<KeyPoint> KeyPointHelper::getLocalMax(const DoubleMatrix& img, const std::vector<int>& windowSize, double threshold)
{
	int offsetY = windowSize[0] / 2;
	int offsetX = windowSize[1] / 2;
	std::vector<KeyPoint> localMaxPoints;
	int height = img.getHeight();
	int width = img.getWidth();
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			bool isLocalMax = true;
			double localMax = img.get(y, x);
			for (int u = -offsetY; u <= offsetY && isLocalMax; u++) {
				for (int v = -offsetX; v <= offsetX && isLocalMax; v++) {
					if (u != 0 || v != 0) {
						isLocalMax = localMax > img.get(y + u, x + v);
					}
				}
			}

			if (isLocalMax && localMax > threshold) {
				localMaxPoints.push_back(KeyPoint(x, y, localMax));
			}
		}
	}

	return localMaxPoints;
}

std::vector<KeyPoint> KeyPointHelper::getKeyPoints(const DoubleMatrix& img, double threshold)
{
	std::vector<KeyPoint> output;
	int height = img.getHeight();
	int width = img.getWidth();
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			double p = img.at(y, x);
			if (p > threshold) output.push_back(KeyPoint(x, y, p));
		}
	}
	return output;
}
