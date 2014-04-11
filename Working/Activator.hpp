#ifndef ACTIVATOR_H
#define ACTIVATOR_H

#include <math.h>

class LinearActivator
{
public:
    float Activate(float x) const { return x; }
    float Derivative(float) const { return 1.0; }
};

class SigmoidActivator
{
public:
    float Activate(float x) const { return 1.0f/( 1.0f + exp(-x)); }
    float Derivative(float y) const { return y * (1.0f - y); }
};

#endif // ACTIVATOR_H
