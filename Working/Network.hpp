#ifndef TRAININGNETWORK_HPP
#define TRAININGNETWORK_HPP
#include "BaseNetwork.hpp"

#include "ImageTransform.hpp"

template <int mode, class InputActivatorClass,class HiddenActivatorClass,class OutputActivatorClass>
class Network : public BaseNetwork<mode, InputActivatorClass, HiddenActivatorClass, OutputActivatorClass>
{

};

template <class InputActivatorClass,class HiddenActivatorClass,class OutputActivatorClass>
class Network<1, InputActivatorClass, HiddenActivatorClass, OutputActivatorClass> : public BaseNetwork<1, InputActivatorClass, HiddenActivatorClass, OutputActivatorClass>
{
protected:
    vector<float> ErrorBuffer;

    float SetError(const float* actual, const float* expected)
    {
        float result = 0.0f;
        for(int i = 0; i < this->OutputSize; i++)
        {
            float err = expected[i] - actual[i];
            ErrorBuffer[i] =  err;
            result += err * err;
        }


        this->OutputLayer.SetErrors(&ErrorBuffer[0]);
        this->HOConnector.SetErrors(this->OutputLayer.GetErrors());
        this->HiddenLayer.SetErrors(this->HOConnector.GetErrors());
        this->IHConnector.SetErrors(this->HiddenLayer.GetErrors());
        this->InputLayer.SetErrors(this->IHConnector.GetErrors());


        return result / this->OutputSize;
    }

    void Learn(float LearningRate, float Momentum)
    {
        this->HOConnector.Learn(this->OutputLayer.GetErrors(), this->HiddenLayer.GetOutput(), LearningRate, Momentum);
        this->IHConnector.Learn(this->HiddenLayer.GetErrors(), this->InputLayer.GetOutput(), LearningRate, Momentum);
    }

    float TrainSample(const float* input, const float* output, float LearningRate, float Momentum)
    {
        const float* actual = this->Run(input);
        float result = SetError(actual, output);

        Learn(LearningRate, Momentum);


        return result;
    }

    class NetConv : public Convertor
    {

    public:
        Network<1, InputActivatorClass, HiddenActivatorClass, OutputActivatorClass>* Net;

        const float* Convert(const float* const from)
        {
            return Net->Run(from);
        }
    };
    NetConv Conv;
public:


    Network(int inputSize, int hiddenSize, int outputSize, float jitter)
        :BaseNetwork<1, InputActivatorClass, HiddenActivatorClass, OutputActivatorClass>(inputSize, hiddenSize, outputSize, jitter)
        ,ErrorBuffer(outputSize)
    {
        Conv.Net = this;
    }

    float Train(SampleSet* data, float LearningRate, float Momentum)
    {
        float result = 0.0f;
        for (int i = 0; i < data->Samples(); i ++)
        {
            result += TrainSample(&data->Input()[data->InputSize() * i], &data->Output()[data->OutputSize() * i], LearningRate, Momentum);
        }
        return result / data->Samples();
    }

    Convertor* Convertor()
    {
        return &Conv;
    }
};

#endif // TRAININGNETWORK_HPP
