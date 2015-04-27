

#include <stdio.h>
#include "MTNFile.h"

void
MTNFile::Print()
{
    printf("magic             : %c%c%c%c\n",
           magic[0], magic[1], magic[2], magic[3]);
    printf("name              : %s\n", GetName());
    printf("author            : %s\n", GetAuthor());
    printf("design            : %s\n", GetRobotDesign());
    printf("numKeyFrames      : %d\n", GetNumKeyFrames());
    printf("frameRate         : %d\n", GetFrameRate());
    printf("numJoints         : %d\n", GetNumJoints());

    for (int i = 0; i < GetNumJoints(); i++)
        printf("locator[%2d]       : %s\n", i, GetLocator(i));

    printf("dataType          : %d\n", GetDataType());
    printf("secNum3           : %d\n", (GetSection3())->sectionNum);
    printf("secSize3          : %d\n", (GetSection3())->sectionSize);
    printf("eachKeyFrameSize  : %d\n", GetEachKeyFrameSize());
    printf("totalKeyFrameSize : %d\n", GetTotalKeyFrameSize());

    for (int i = 0; i < GetNumKeyFrames() - 1; i++) {
        PrintKeyFrame(i);
        printf("i[%d] %d\n", i, GetNumInterpolate(i));
    }
    PrintKeyFrame(GetNumKeyFrames() - 1);
}

inline double
degrees(double rad)
{
    return 180.0 * rad / 3.14159265358979323846; 
}

double
microRadianToDegree( int microRadian )
{
    return degrees( ((double) microRadian) / 1000000.0 );
}

void
MTNFile::PrintKeyFrame(int index)
{
    int* keyFrame = (int*)GetKeyFrame(index);

    printf("k[%02d] %d %d %d : ",
	   index, keyFrame[0], keyFrame[1], keyFrame[2]);
    keyFrame += 3;

    for (int j = 0; j < GetNumJoints(); j++) {
        printf("%4.1f ", microRadianToDegree(keyFrame[j]));
    }
    printf("\n");
}
