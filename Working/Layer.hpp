#ifndef LAYER_HPP
#define LAYER_HPP

#include <vector>
#include <fstream>
#include <iostream>

using namespace std;

template <int mode, class ActivatorType>
class Layer
{
protected:
    ActivatorType Activator;
    int Neurons;
    std::vector<float> Output;

public:
    Layer(int neurons)
        :Neurons(neurons)
        ,Output(Neurons + 1, 1.0f)
    {
    }

    Layer(std::ifstream& stream)
    {
        stream.read( (char*)&Neurons, sizeof(Neurons));
        Output.resize(Neurons + 1, 1.0f);
    }

    int GetNeurons() const { return Neurons; }
    const float* GetOutput() const { return &Output[0]; }

    void Run(const float* const input)
    {
        #pragma omp parallel for schedule(dynamic)
        for(int n = 0; n < Neurons; n++)
        {
            Output[n] = Activator.Activate(input[n]);
        }
    }
};


template <class ActivatorType>
class Layer<1, ActivatorType> : public Layer<0, ActivatorType>
{
protected:
    std::vector<float> Errors;
public:
    Layer(int neurons)
        :Layer<0, ActivatorType>(neurons)
        ,Errors(neurons, 0.0f)
    {
    }

    void SetErrors(const float* const err)
    {
        #pragma omp parallel for schedule(dynamic)
        for(int n = 0; n < this->Neurons; n++)
        {
            Errors[n] = err[n] * this->Activator.Derivative(this->Output[n]);
        }
    }

    const float* GetErrors() const { return &Errors[0]; }
};
#endif // LAYER_HPP
