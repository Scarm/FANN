#ifndef IMAGETRANSFORM_H
#define IMAGETRANSFORM_H

#include <QImage>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;


const int window = 64;

//содержит только входы
class DataSet
{
protected:
    int inputSize;
    int samples;
public:
    virtual float* Input() = 0;
    int InputSize() { return inputSize; }

    int Samples() { return samples; }

    DataSet(int inputSize, int size)
        :inputSize(inputSize)
        ,samples(size)
    {

    }

    DataSet(const DataSet& src)
        :inputSize(src.inputSize)
        ,samples(src.samples)
    {

    }
};

// Содержит пары вход-выход
class SampleSet : public DataSet
{
protected:
    int outputSize;
public:
    virtual float* Output() = 0;
    int OutputSize() { return outputSize; }

    SampleSet(int inputSize,int outputSize, int size)
        :DataSet(inputSize, size)
        ,outputSize(outputSize)
    {

    }

    SampleSet(const SampleSet& src)
        :DataSet(src.inputSize, src.samples)
        ,outputSize(src.outputSize)
    {
    }
};


class SupervisedSet : public SampleSet
{
protected:
    vector<float> input;
    vector<float> output;
public:
    SupervisedSet(int inputSize,int outputSize, int size)
        :SampleSet(inputSize, outputSize, size)
        ,input(inputSize * size)
        ,output(outputSize * size)
    {

    }

    SupervisedSet(const SupervisedSet& src)
        :SampleSet(src.inputSize, src.outputSize, src.samples)
        ,input(src.input)
        ,output(src.output)
    {

    }

    float* Input()
    {
        return &input[0];
    }

    float* Output()
    {
        return &output[0];
    }
};


class EncoderSampleSet : public SampleSet
{
protected:
    vector<float> Data;

public:
    EncoderSampleSet(int inputSize, int size)
        :SampleSet(inputSize, inputSize, size)
        ,Data(inputSize * size)
    {
    }

    float* Input()
    {
        return &Data[0];
    }

    float* Output()
    {
        return &Data[0];
    }
};

class Convertor
{
public:
    virtual const float* Convert(const float* const form) = 0;
};


class ImageSet
{
private:

    struct ImageData
    {
        int Height;
        int Width;
        vector<float> Data;

        ImageData(int height, int width)
            :Height(height)
            ,Width(width)
            ,Data(height*width, 0.0)
        {

        }
    };

    struct SampleTask
    {
        int X, Y;
        ImageData* Set;
        int order;

        SampleTask(int x, int y, ImageData& src)
            :X(x)
            ,Y(y)
            ,Set(&src)
        {
            order = rand();
        }
    };



    struct TaskSort {
      bool operator() (const SampleTask& i,const SampleTask& j)
      {
          return (i.order < j.order);
      }
    };


    static inline float sigma(const float x, const float alpha) { return 1.0f/( 1.0f + exp(-alpha * x)); }

    static inline vector<QImage> ScaledSet(QImage& src, double scaleRate, int steps)
    {
        vector<QImage> images(steps);
        int height = src.height();

        double scale = 1.0;
        for(int i = 0; i < steps; i++)
        {
            images[i] = src.scaledToHeight( int (height / scale + 0.5 ) );
            scale *= scaleRate;
        }
        return images;
    }

    static inline ImageData GetData(QImage& src)
    {
        int Height = src.height();
        int Width = src.width();

        ImageData result(Height, Width);

        for(int x = 0; x < Width; x++)
        {
            for(int y = 0; y < Height; y++)
            {
                result.Data[y * Width + x] = qRed(src.pixel(x,y)) / 255.0f;
            }
        }

        return result;
    }

    static inline QImage FromData(ImageData& src)
    {
        int Height = src.Height;
        int Width = src.Width;
        QImage result(Width,Height,QImage::Format_RGB32);
        for(int x = 0; x < Width; x++)
        {
            for(int y = 0; y < Height; y++)
            {
                float val = src.Data[y * Width + x];
                val = std::max(val, 0.0f);
                val = std::min(val, 1.0f);
                uint value = 255 * val;
                QRgb pixel = qRgb(value, value, value);
                result.setPixel(x, y, pixel);
            }
        }

        return result;
    }

    static inline float AvgDisp(ImageData& src, int window)
    {
        int Height = src.Height;
        int Width = src.Width;

        float result = 0.0f;
        for(int x = 0; x < Width - window + 1; x++)
        {
            for(int y = 0; y < Height - window + 1; y++)
            {
                int minX = x;
                int maxX = x + window;
                int minY = y;
                int maxY = y + window;

                float M = 0.0;
                float M2 = 0.0;
                for(int i = minX; i < maxX; i++)
                {
                    for(int j = minY; j < maxY; j++)
                    {
                        float val = src.Data[j * Width + i];
                        M += val;
                        M2+= val * val;
                    }
                }
                M /= (maxX - minX)*(maxY - minY);
                M2 /= (maxX - minX)*(maxY - minY);
                float D = M2 - M*M;
                result += D;
            }
        }
        return result / ((Height - window) *(Width - window));
    }

    static inline ImageData Normalize(ImageData& src, int window)
    {

        int Height = src.Height;
        int Width = src.Width;
        ImageData dst(Height, Width);

        float M0 = 0.0f;
        for(int x = 0; x < Width; x++)
        {
            for(int y = 0; y < Height; y++)
            {
                M0 += src.Data[y * Width + x];
            }
        }
        M0 /= Height * Width;

        for(int x = 0; x < Width; x++)
        {
            for(int y = 0; y < Height; y++)
            {
                int minX = x - window/2;
                int maxX = x + (window+1)/2;
                int minY = y - window/2;
                int maxY = y + (window+1)/2;

                float M = 0.0;

                for(int i = minX; i < maxX; i++)
                {
                    for(int j = minY; j < maxY; j++)
                    {
                        float val = M0;
                        if ( i >= 0 && i < Width && j > 0 && j<Height )
                        {
                            val = src.Data[j * Width + i];
                        }
                        M += val;
                    }
                }
                M /= window * window;
                dst.Data[y * Width + x] = (src.Data[y * Width + x] - M);
            }
        }


        float M = 0.0f;
        for(int x = 0; x < Width; x++)
        {
            for(int y = 0; y < Height; y++)
            {
                M += dst.Data[y * Width + x];
            }
        }
        M /= Height * Width;

        cout << "M = " << M << endl;

        float D = 0.0f;
        for(int x = 0; x < Width; x++)
        {
            for(int y = 0; y < Height; y++)
            {
                D += (dst.Data[y * Width + x] - M) * (dst.Data[y * Width + x] - M);
            }
        }
        D /= Height * Width;
        cout << "D = " << D << endl;


        float alpha = 1e-4f;
        float err = 0.0f;
        while( err < 0.5)
        {
            alpha *= 1.01;
            err = 0.0;
            for(int x = 0; x < Width; x++)
            {
                for(int y = 0; y < Height; y++)
                {
                    double e = alpha * dst.Data[y * Width + x] - (sigma(dst.Data[y * Width + x], alpha) - 0.5);
                    err+= e * e;
                }
            }
            err =  sqrt(err / (Height * Width));
        }
        cout << alpha << " " << err << endl;

        for(int x = 0; x < Width; x++)
        {
            for(int y = 0; y < Height; y++)
            {
                dst.Data[y * Width + x] = sigma(dst.Data[y * Width + x], alpha);
            }
        }

        return dst;
    }

    static inline vector<SampleTask> CreateTasks(ImageData& src, int window)
    {
        int X = src.Width - window + 1;
        int Y = src.Height - window + 1;
        vector<SampleTask> result;
        result.reserve(X * Y);
        for (int x = 0; x < X; x++)
        {
            for(int y = 0; y < Y; y++)
            {
                result.push_back(SampleTask(x, y, src));
            }
        }
        return result;
    }

    static inline void ExtractData(ImageData& src, int X, int Y, int window, float* dst)
    {
        for(int x = 0; x < window; x++)
        {
            for(int y = 0; y < window; y++)
            {
                dst[y * window + x] = src.Data[ (y + Y)* src.Width + (x + X)];
            }
        }
    }

public:

    static inline QImage Normalize(QImage& src, int window)
    {
        ImageData data = GetData(src);
        ImageData res = Normalize(data, window);
        return FromData(res);
    }

    static inline float AvgDisp(QImage& src, int window)
    {
        ImageData data = GetData(src);
        return AvgDisp(data, window);
    }

    static inline EncoderSampleSet CreateEncoderSamples(QImage& src)
    {
        const double scaleRate = exp( 0.25 * log(2) );
        const int steps = 9;
        const int window = 16;

        vector<QImage> images = ScaledSet(src, scaleRate, steps);
        vector<SampleTask> tasks;

        for(int i = 0; i < steps; i++)
        {
            ImageData data = GetData(images[i]);
            data = Normalize(data,window);
            vector<SampleTask> tmp = CreateTasks(data, window);
            tasks.insert(tasks.end(), tmp.begin(), tmp.end());
        }
        sort(tasks.begin(),tasks.end(), TaskSort());

        EncoderSampleSet result(window * window, tasks.size());
        float* data = result.Input();
        int size = result.InputSize();

        for(int i = 0; i < (int)tasks.size(); i++)
        {
            SampleTask& task = tasks[i];
            ExtractData(*task.Set, task.X, task.Y, window, &data[size * i]);

        }
        return result;
    }

    static inline SupervisedSet CreateSupervisedSamples(QImage& src, QImage& bw)
    {
        //const int window = 40;
        ImageData data = GetData(src);
        data = Normalize(data,window);
        ImageData outputs = GetData(bw);
        vector<SampleTask> tasks = CreateTasks(data, window);
        sort(tasks.begin(),tasks.end(), TaskSort());

        SupervisedSet result(window * window, window * window, tasks.size());


        float* input = result.Input();
        int inputSize = result.InputSize();
        float* output = result.Output();
        int outputSize = result.OutputSize();
        for(int i = 0; i < (int)tasks.size(); i++)
        {
            SampleTask& task = tasks[i];
            ExtractData(data,    task.X, task.Y, window, &input[inputSize * i]);
            ExtractData(outputs, task.X, task.Y, window, &output[outputSize * i]);
        }

        return result;
    }

    QImage static inline Convert(QImage& src, Convertor* conv)
    {
        //const int window = 40;
        ImageData data = GetData(src);
        vector<SampleTask> tasks = CreateTasks(data, window);


        int Width = src.width();
        int Height = src.height();


        vector<float> input(window * window);

        vector<float> dst(Height * Width, 0.0f);
        vector<int> divisors(Height * Width, 0);

        for(int i = 0; i < (int)tasks.size(); i++)
        {
            SampleTask& task = tasks[i];
            ExtractData(*task.Set, task.X, task.Y, window, &input[0]);
            const float* output = conv->Convert(&input[0]);


            for(int x = 0; x < window; x++)
            {
                for(int y = 0; y < window; y++)
                {
                    dst[(task.Y + y) * Width + task.X + x] += output[y * window + x] * 255.0f;
                    divisors[(task.Y + y) * Width + task.X + x]++;
                    //QRgb pixel= qRgb(value, value, value);
                    //result.setPixel(task.X + x,task.Y + y,pixel);
                }
            }
        }

        QImage result(Width, Height, QImage::Format_RGB32);
        for(int x = 0; x < Width; x++)
        {
            for(int y = 0; y < Height; y++)
            {
                float value = dst[y * Width + x] / divisors[y * Width + x];
                QRgb pixel= qRgb(value, value, value);
                result.setPixel(x,y,pixel);
            }
        }


        return result;
    }


    static inline QImage ToBlackWhite(QImage& src)
    {
        int Height = src.height();
        int Width = src.width();


        QImage result(Width, Height, QImage::Format_RGB32);

        for(int x = 0; x < Width; x++)
        {
            for(int y = 0; y < Height; y++)
            {
                QRgb pixel;
                if (qRed(src.pixel(x,y)) == 255 && qGreen(src.pixel(x,y)) == 0 && qBlue(src.pixel(x,y)) == 0)
                {
                    pixel = qRgb(255, 255, 255);
                }
                else
                {
                    pixel = qRgb(0, 0, 0);
                }

                result.setPixel(x,y,pixel);
            }
        }

        return result;
    }
};

#endif // IMAGETRANSFORM_H
