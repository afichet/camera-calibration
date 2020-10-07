#ifndef UTIL_H_
#define UTIL_H_

float alpha(float x0, float x1, float x)
{
    return (x - x0) / (x1 - x0);
}


float lerp(float a, float b, float t)
{
    return a + t * (b - a);
}


float interp(float x, float x0, float x1, float y0, float y1)
{
    return lerp(y0, y1, alpha(x0, x1, x));
}

#endif // UTIL_H_
