#include <gui/GUI.h>
#include <gui/actions/ActionManager.h>
#include <core/dataset/UndoStack.h>
#include <core/dataset/DataSetContainer.h>
#include <core/dataset/scene/SelectionSet.h>
#include <core/dataset/scene/SceneRoot.h>
#include <core/dataset/animation/AnimationSettings.h>
#include <gui/viewport/input/NavigationModes.h>
#include <gui/viewport/input/ViewportInputMode.h>
#include <gui/viewport/input/ViewportInputManager.h>
#include <gui/actions/ViewportModeAction.h>
#include <gui/mainwin/MainWindow.h>

#include <fstream>
#include "ActionManager.h"

namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui)

void ActionManager::MgSnCu4(int x, int y, int z, float acell) {
    
    float b = acell * std::sqrt(3) / 4 / 1.11;
    
    std::vector<float> posX, posY, posZ, typePos, result;
    
    posX.push_back(0); posY.push_back(0); posZ.push_back(0);
    posX.push_back(acell / 2); posY.push_back(0); posZ.push_back(acell / 2);
    posX.push_back(acell / 2); posY.push_back(acell / 2); posZ.push_back(0);
    posX.push_back(0); posY.push_back(acell / 2); posZ.push_back(acell / 2);
    
    posX.push_back(acell / 4); posY.push_back(acell / 4); posZ.push_back(acell / 4);
    posX.push_back(acell * 3 / 4); posY.push_back(acell * 3 / 4); posZ.push_back(acell / 4);
    posX.push_back(acell / 4); posY.push_back(acell * 3 / 4); posZ.push_back(acell * 3 / 4);
    posX.push_back(acell * 3 / 4); posY.push_back(acell / 4); posZ.push_back(acell * 3 / 4);
    
    posX.push_back(acell * 3 / 4 - b / std::sqrt(8)); posY.push_back(acell * 3 / 4 - b / std::sqrt(8)); posZ.push_back(acell * 3 / 4 - b / std::sqrt(8));
    posX.push_back(acell * 3 / 4 - b / std::sqrt(8)); posY.push_back(acell * 3 / 4 + b / std::sqrt(8)); posZ.push_back(acell * 3 / 4 + b / std::sqrt(8));
    posX.push_back(acell * 3 / 4 + b / std::sqrt(8)); posY.push_back(acell * 3 / 4 - b / std::sqrt(8)); posZ.push_back(acell * 3 / 4 + b / std::sqrt(8));
    posX.push_back(acell * 3 / 4 + b / std::sqrt(8)); posY.push_back(acell * 3 / 4 + b / std::sqrt(8)); posZ.push_back(acell * 3 / 4 - b / std::sqrt(8));
    
    
    for (int i = 0; i < 4; i++) {
        posX.push_back(posX[8 + i] - acell / 2); posY.push_back(posY[8 + i] - acell / 2); posZ.push_back(posZ[8 + i]);
    }
    
    for (int i = 0; i < 4; i++) {
        posX.push_back(posX[8 + i]); posY.push_back(posY[8 + i] - acell / 2); posZ.push_back(posZ[8 + i] - acell / 2);
    }
    
    for (int i = 0; i < 4; i++) {
        posX.push_back(posX[8 + i] - acell / 2); posY.push_back(posY[8 + i]); posZ.push_back(posZ[8 + i] - acell / 2);
    }
    
    for (int i = 0; i < 4; i++) {
        typePos.push_back(1);
    }
    
    for (int i = 0; i < 4; i++) {
        typePos.push_back(2);
    }
    
    for (int i = 0; i < 16; i++) {
        typePos.push_back(3);
    }
    
    int atnum = 0;
    for (int i1 = 0; i1 < x; i1++) {
        for (int i2 = 0; i2 < y; i2++) {
            for (int i3 = 0; i3 < z; i3++) {
                for (int i = 0; i < 24; i++) {
                    atnum++;
                    result.push_back(posX[i] + (i1 + 1) * acell);
                    result.push_back(posY[i] + (i2 + 1) * acell);
                    result.push_back(posZ[i] + (i3 + 1) * acell);
                    result.push_back(typePos[i]);
                }
            }
        }
    }
    // generate output datafile
    std::ofstream output;
    output.open("MgSnCu4.xyz");
    
    output << atnum << std::endl << "Atoms" << std::endl;
    
    for (int i = 0; i < atnum; i++) {
        
        char buff[100];
        std::snprintf(buff, sizeof(buff), "%d      %E    %E    %E\r\n",
                      (int)result[4 * i + 3], result[4 * i], result[4 * i + 1], result[4 * i + 2]);
        std::string str = buff;
        output << str;
    }
    output.close();
    
}
}


