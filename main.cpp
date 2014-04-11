#include <QImage>
#include <QTime>
#include <QDir>
#include <iostream>
#include <math.h>
#include <vector>
#include "ImageTransform.hpp"
#include "Working/Network.hpp"

using namespace std;

void FromRedGrayToBlackWhite()
{
    QString from = "/home/user/FANN/FANN/Outputs/";
    QString to = "/home/user/FANN/FANN/BlackWhite/";
    {
        QStringList fileList = QDir(from).entryList(QString("*.png").split(" "),QDir::Files);
        int listSize = fileList.size();
        for(int i = 0; i < listSize; i++)
        {
            QImage src(from + fileList[i]);
            QImage dst = ImageSet::ToBlackWhite(src);
            dst.save(to + fileList[i],"PNG");
        }
    }
}

int main()
{
    QTime time;
    time.start();
    //QString inputsPath = "/home/user/Algorithm/bases/Gallery_handeyes/";
    QString inputsPath = "/home/user/Algorithm/bases/FERET_Gallery/";
    //QString inputsPath = "/home/user/Algorithm/bases/Query1/";

    QString outputPath = "/home/user/FANN/Normalized/";
    QStringList fileList = QDir(inputsPath).entryList(QString("*.jpg").split(" "),QDir::Files);
    int listSize = fileList.size();
    cout << "files = " << listSize << endl;

    //int i = 0;
    for( int i = 0; i < 10; i++)
    {
        QImage input(inputsPath + fileList[i]);
        ImageSet::Normalize(input, 40).save(outputPath + fileList[i], "JPG");
    }

    /*
    for (int window = 4; window < 177; window += 16)
    {
        float avgD = 0.0f;
        #pragma omp parallel for schedule(dynamic) reduction(+:avgD)
        for( int i = 0; i < listSize; i++)
        {
            QImage input(inputsPath + fileList[i]);
            avgD += ImageSet::AvgDisp(input, window);
        }
        cout << window << ";" << avgD / listSize << endl;
    }
    */

//    QTime time;
//    time.start();

//    Network<1, LinearActivator, SigmoidActivator, SigmoidActivator> net(window * window, 2 * window, window * window,0.1);

//    QString inputsPath = "/home/user/FANN/FANN/Inputs/";
//    QString bwPath = "/home/user/FANN/FANN/BlackWhite/";
//    QString outPath = "/home/user/FANN/FANN/Outputs/";

//    QStringList fileList = QDir(inputsPath).entryList(QString("*.png").split(" "),QDir::Files);
//    int listSize = fileList.size();
//    cout << "files = " << listSize << endl;

//    int epoch = 0;
//    float error = 1e10;
//    const int bufferSize = 10;
//    vector<float> errorBuffer(bufferSize, 0.0f);


//    float avgError = 0.0f;

//    while(avgError / min(bufferSize, epoch) > 0.04 || epoch == 0 )
//    {


//        cout << "epoch = " << epoch << flush;
//        QImage input(inputsPath + fileList[epoch % listSize]);
//        QImage bw(bwPath + fileList[epoch % listSize]);

//        SupervisedSet set = ImageSet::CreateSupervisedSamples(input, bw);

//        error = net.Train(&set, 0.05f, 0.05f);

//        avgError -= errorBuffer[epoch % bufferSize];
//        errorBuffer[epoch % bufferSize] = error;
//        avgError += errorBuffer[epoch % bufferSize];

//        cout <<  " error = " << error << " avg error = " << avgError / min(bufferSize, epoch + 1) << flush;

//        QImage out = ImageSet::Convert(input, net.Convertor());
//        cout << " " << out.save(outPath + fileList[epoch % listSize],"PNG") << endl;
//        epoch = epoch + 1;
//    }

//    cout<< "network trained!"<<endl;

//    /*
//    for(int i = 0; i < listSize; i++)
//    {
//        cout << i << " " << flush;
//        QImage input(inputsPath + fileList[i]);
//        QImage out = ImageSet::Convert(input, net.Convertor());
//        cout << out.save(outPath + fileList[i],"PNG") << endl;
//    }
//    */

//    /*

//    EncoderSampleSet set = ImageSet::CreateEncoderSamples(src);
//    SampleSet* base = &set;
//    cout << set.Samples() << endl;

//    Network< Layer<LinearActivator>, Layer<SigmoidActivator>, Layer<SigmoidActivator> > net(256,128,256,0.1);
//    */


//    //net.Train(base);
//    //Transformer& tr = net.Transformer;


    cout<<"finish "<< time.elapsed()<<endl;
}
