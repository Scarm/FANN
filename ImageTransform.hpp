#ifndef IMAGETRANSFORM_H
#define IMAGETRANSFORM_H

#include <QImage>
#include <vector>
#include <iostream>
#include <algorithm>

using namespace std;


class SampleSet
{
    int inputSize;
    int outputSize;
    int samples;
public:
    virtual float* Input() = 0;
    int InputSize() { return inputSize; }

    virtual float* Output() = 0;
    int OutputSize() { return outputSize; }

    int Samples() { return samples; }

    SampleSet(int window, int size)
        :inputSize(window * window)
        ,outputSize(window * window)
        ,samples(size)
    {

    }
};

class EncoderSampleSet : public SampleSet
{

    vector<float> Data;

public:
    EncoderSampleSet(int window, int size)
        :SampleSet(window, size)
        ,Data(window * window * size)
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

    static inline ImageData Normalize(ImageData& src, int window)
    {

        int Height = src.Height;
        int Width = src.Width;
        ImageData dst(Height, Width);

        for(int x = 0; x < Width; x++)
        {
            for(int y = 0; y < Height; y++)
            {
                int minX = std::max(x - window/2, 0);
                int maxX = std::min(x + (window+1)/2, Width);
                int minY = std::max(y - window/2, 0);
                int maxY = std::min(y + (window+1)/2, Height);

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
                M2 =  sqrt(M2/(maxX - minX)*(maxY - minY));
                float D = M2 - M*M;

                if (D == 0)
                {
                    dst.Data[y * Width + x] = 0.0f;
                }
                else
                {
                    dst.Data[y * Width + x] = (src.Data[y * Width + x] - M) / D;
                }
            }
        }


        float alpha = 50.0f;
        float err = 0.0f;
        while( err < 0.15)
        {
            alpha *= 1.01;
            err = 0.0;
            for(int x = 0; x < Width; x++)
            {
                for(int y = 0; y < Height; y++)
                {
                    double e = dst.Data[y * Width + x] - (sigma(dst.Data[y * Width + x], alpha) - 0.5);
                    err+= e * e;
                }
            }
            err =  sqrt(err / (Height * Width));
        }

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

    static inline EncoderSampleSet CreateEncoderSamples(QImage& src)
    {
        const double scaleRate = exp( 0.25 * log(2) );
        const int steps = 9;
        const int window = 16;

        vector<QImage> images = ImageSet::ScaledSet(src, scaleRate, steps);
        vector<SampleTask> tasks;

        for(int i = 0; i < steps; i++)
        {
            ImageData data = ImageSet::GetData(images[i]);
            data = ImageSet::Normalize(data,window);
            vector<SampleTask> tmp = ImageSet::CreateTasks(data, window);
            tasks.insert(tasks.end(), tmp.begin(), tmp.end());
        }
        sort(tasks.begin(),tasks.end(), TaskSort());

        EncoderSampleSet result(window, tasks.size());
        float* data = result.Input();
        int size = result.InputSize();

        for(int i = 0; i < (int)tasks.size(); i++)
        {
            SampleTask& task = tasks[i];
            ExtractData(*task.Set, task.X, task.Y, window, &data[size * i]);
        }
        return result;
    }

};

#endif // IMAGETRANSFORM_H
