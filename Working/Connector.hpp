#ifndef CONNECTOR_HPP
#define CONNECTOR_HPP

#include <vector>
#include <fstream>
#include <iostream>

template <int mode>
class Connector
{
protected:
    int InputNeurons;
    int OutputNeurons;
    std::vector<float> Output;
    std::vector<float> Weights;
public:
    Connector(std::ifstream& stream)
    {
        stream.read( (char*)&InputNeurons, sizeof(InputNeurons));
        stream.read( (char*)&OutputNeurons, sizeof(OutputNeurons));
        Output.resize(OutputNeurons);
        Weights.resize((InputNeurons + 1) * OutputNeurons, 0.0f);
        stream.read( (char*)&Weights[0], Weights.size() * sizeof(float));
    }

    Connector(int inputNeurons, int outputNeurons, float jitter)
        :InputNeurons(inputNeurons)
        ,OutputNeurons(outputNeurons)
        ,Output(OutputNeurons)
        ,Weights((InputNeurons + 1) * OutputNeurons, 0.0f)
    {
        for (int o = 0; o < OutputNeurons; o++)
        {
            for (int i = 0; i < InputNeurons + 1; i++)
            {
                Weights[o * (InputNeurons + 1) + i] = jitter * (2.0f * rand() / double(RAND_MAX) - 1.0f);
            }
        }
    }

    const float* GetOutput() const { return &Output[0]; }

    void Run(const float* const input)
    {
        #pragma omp parallel for schedule(dynamic)
        for (int o = 0; o < OutputNeurons; o++)
        {
            Output[o] = 0;
            for (int i = 0; i < InputNeurons + 1; i++)
            {
                Output[o] += input[i] * Weights[o * (InputNeurons + 1) + i];
            }
        }
    }
};

template <>
class Connector<1> : public Connector<0>
{
protected:
    std::vector<float> Errors;
    std::vector<float> Deltas;
public:
    Connector(int inputNeurons, int outputNeurons, float jitter)
        :Connector<0>(inputNeurons, outputNeurons, jitter)
        ,Errors(inputNeurons, 0.0f)
        ,Deltas((InputNeurons + 1) * OutputNeurons, 0.0f)
    {

    }

    void SetErrors(const float* const err)
    {
        #pragma omp parallel for schedule(dynamic)
        for (int i = 0; i < InputNeurons; i++)
        {
            Errors[i] = 0;
            for (int o = 0; o < OutputNeurons; o++)
            {
                Errors[i] += err[o] * Weights[o * (InputNeurons + 1) + i];
            }
        }
    }

    const float* GetErrors() const { return &Errors[0]; }

    void Learn(const float* const err, const float* const input, float LearningRate, float Momentum)
    {
        #pragma omp parallel for schedule(dynamic)
        for (int o = 0; o < OutputNeurons; o++)
        {
            for (int i = 0; i < InputNeurons + 1; i++)
            {
                int pos = o * (InputNeurons + 1) + i;
                Deltas[pos] = LearningRate * (Deltas[pos] * Momentum + (1.0f - Momentum) * err[o] * input[i]);
                Weights[pos] += Deltas[pos];
            }
        }
    }
};
#endif // CONNECTOR_HPP
