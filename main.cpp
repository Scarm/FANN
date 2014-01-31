#include <QImage>
#include <iostream>
#include <math.h>
#include <vector>
#include "ImageTransform.h"

using namespace std;
int main()
{
    const double scaleRate = exp( 0.25 * log(2) );


    QImage src("/home/user/Algorithm/bases/Gallery_handeyes/00001fa010_930831.png");

    vector<QImage> images = ImageSet::ScaledSet(src, scaleRate, 9);

    for(int i = 0; i < 9; i++)
    {
        ImageData data = ImageSet::GetData(images[i]);
        data = ImageSet::Normalize(data,16);
        QImage img = ImageSet::FromData(data);
        img.save(QString("/home/user/FANN/FANN/Data/scale_%0").arg(i),"PNG");
    }

    cout<<"finish"<<endl;
}
