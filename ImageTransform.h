#ifndef IMAGETRANSFORM_H
#define IMAGETRANSFORM_H

#include <QImage>
#include <vector>
#include <iostream>

using namespace std;

class ImageData
{
private:

public:
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

class ImageSet
{
private:
    static inline float sigma(const float x, const float alpha) { return 1.0f/( 1.0f + exp(-alpha * x)); }
public:
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
        while( err < 0.03)
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
            err =  err / (Height * Width);
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

};

#endif // IMAGETRANSFORM_H
