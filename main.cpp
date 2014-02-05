#include <QImage>
#include <QTime>
#include <iostream>
#include <math.h>
#include <vector>
#include "ImageTransform.hpp"

using namespace std;
int main()
{

    QImage src("/home/user/Algorithm/bases/Gallery_handeyes/00001fa010_930831.png");

    QTime time;
    time.start();

    EncoderSampleSet set = ImageSet::CreateEncoderSamples(src);
    SampleSet* base = &set;
    cout << set.Samples() << endl;
    cout<<"finish "<< time.elapsed()<<endl;

}
