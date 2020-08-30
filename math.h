#pragma once

#define PI 3.14159f

float mapToNewRange(float x, float minBefore, float maxBefore, float minAfter, float maxAfter) {

	float beforeRange = maxBefore - minBefore;
	float afterRange = maxAfter - minAfter;
	return x / (beforeRange / afterRange) + minAfter;

}