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

void ActionManager::A1B2(int x, int y, int z, float acell) {
   //const float acell = std::sqrt(4) / 3;
       // create a vector to store coordinates
    std::vector<float> posX, posY, posZ, typePos, result;
    posX.push_back(0); posY.push_back(0); posZ.push_back(0); typePos.push_back(1);
    posX.push_back(0); posY.push_back(acell / 2); posZ.push_back(acell / 2); typePos.push_back(2);
    
    int atnum = 0;
    for (int i1 = 0; i1 < x; i1++) {
        for (int i2 = 0; i2 < y; i2++) {
            for (int i3 = 0; i3 < z; i3++) {
                for (int i = 0; i < 2; i++) {
                    atnum++;
                    result.push_back(posX[i] + (i1+1) * acell);
                    result.push_back(posY[i] + (i2+1) * acell);
                    result.push_back(posZ[i] + (i3+1) * acell);
                    result.push_back(typePos[i]);
                }
            }
        }
    }
    // generate output datafile
    std::ofstream output;
    output.open("A1B2.xyz");
 output << atnum << std::endl << "Atoms" << std::endl;
    for (int i = 0; i < atnum; i++) {
        int typeatom(0);
        if (i <= atnum) {
            typeatom = 1;
        }
        
        char buff[100];
        std::snprintf(buff, sizeof(buff), "%d      %E    %E    %E\r\n",
                      (int)result[4*i+3], result[4*i], result[4*i+1], result[4*i+2]);
        std::string str = buff;
        output << str;
    }
    output.close();
}
}
