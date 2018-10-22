/* 
 * File:   FCC.cpp
 * Author: shulei
 * Copyright (C) 2017 xus6@miamioh.edu
 * Created on November 14, 2017, 3:19 PM
 */

#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include "FCC.h"

FCC::FCC(int x, int y, int z) {
    this->x = x;
    this->y = y;
    this->z = z;
}

FCC::FCC(const FCC& orig) {
}

FCC::~FCC() {
}

std::string FCC::generateData() {
    const float acell = std::sqrt(4) / 3;
    // create a vector to store coordinates
    std::vector<float> posX, posY, posZ, result;
    posX.push_back(0); posY.push_back(0); posZ.push_back(0);
    posX.push_back(acell / 2); posY.push_back(acell / 2); posZ.push_back(acell / 2);
    posX.push_back(0); posY.push_back(acell); posZ.push_back(acell);
    posX.push_back(acell); posY.push_back(0); posZ.push_back(acell);
    
    int atnum = 0;
    for (int i1 = 0; i1 < x; i1++) {
        for (int i2 = 0; i2 < y; i2++) {
            for (int i3 = 0; i3 < z; i3++) {
                for (int i = 0; i < 4; i++) {
                    atnum++;
                    result.push_back(posX[i] + i1 * acell);
                    result.push_back(posY[i] + i2 * acell);
                    result.push_back(posZ[i] + i3 * acell);
                }
            }
        }
    }
    // generate output datafile
    std::ofstream output;
    output.open("fcc.xyz");
    for (int i = 0; i < atnum; i++) {
        int typeatom(0);
        if (i <= atnum) {
            typeatom = 1;
        }
    
        char buff[100];
        std::snprintf(buff, sizeof(buff), "%d      %E    %E    %E\r\n",
                typeatom, result[3*i], result[3*i+1], result[3*i+2]);
        std::string str = buff;
        output << str;
    }
    return "fcc.xyz";
}

