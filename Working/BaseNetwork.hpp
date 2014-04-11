#ifndef NETWORK_HPP
#define NETWORK_HPP

#include "Activator.hpp"
#include "Layer.hpp"
#include "Connector.hpp"

template <int mode, class InputActivatorClass,class HiddenActivatorClass,class OutputActivatorClass>
class BaseNetwork
{
protected:
    int InputSize;
    int HiddenSize;
    int OutputSize;

    Layer<mode, InputActivatorClass> InputLayer;
    Layer<mode, HiddenActivatorClass>  HiddenLayer;
    Layer<mode, OutputActivatorClass>  OutputLayer;

    Connector<mode> IHConnector;
    Connector<mode> HOConnector;

    const float* Run(const float* const input)
    {
        InputLayer.Run(input);
        IHConnector.Run(InputLayer.GetOutput());
        HiddenLayer.Run(IHConnector.GetOutput());
        HOConnector.Run(HiddenLayer.GetOutput());
        OutputLayer.Run(HOConnector.GetOutput());
        return OutputLayer.GetOutput();
    }

public:
    BaseNetwork(std::ifstream& stream)
        :InputLayer(stream)
        ,HiddenLayer(stream)
        ,OutputLayer(stream)
        ,IHConnector(stream)
        ,HOConnector(stream)
    {
        stream.read( (char*)&InputSize, sizeof(InputSize) );
        stream.read( (char*)&HiddenSize, sizeof(HiddenSize) );
        stream.read( (char*)&OutputSize, sizeof(OutputSize) );
    }

    BaseNetwork(int inputSize, int hiddenSize, int outputSize, float jitter)
        :InputSize(inputSize)
        ,HiddenSize(hiddenSize)
        ,OutputSize(outputSize)
        ,InputLayer(inputSize)
        ,HiddenLayer(hiddenSize)
        ,OutputLayer(outputSize)
        ,IHConnector(inputSize, hiddenSize, jitter)
        ,HOConnector(hiddenSize, outputSize, jitter)
    {

    }

    int GetInputSize() { return InputSize; }
    int GetHiddenSize() { return HiddenSize; }
    int GetOutputSize() { return OutputSize; }
};


#endif // NETWORK_HPP
