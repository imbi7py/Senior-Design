/* 
 * File:   FCC.h
 * Author: shulei
 *
 * Created on November 14, 2017, 3:19 PM
 */
#include <string>

#ifndef FCC_H
#define	FCC_H
class FCC {
public:
    FCC(int x, int y, int z);
    FCC(const FCC& orig);
    virtual ~FCC();
    std::string generateData();
private:
    int x, y, z;
};

#endif	/* FCC_H */

