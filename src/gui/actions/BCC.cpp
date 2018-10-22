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

void ActionManager::BCC(int x, int y, int z, float a) {
    const float acell = a;//std::sqrt(4) / 3;
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
                        result.push_back(posX[i] + (i1+1) * acell);
                        result.push_back(posY[i] + (i2+1) * acell);
                        result.push_back(posZ[i] + (i3+1) * acell);
                    }
                }
            }
        }
        // generate output datafile
        std::ofstream output;
        output.open("bcc.xyz");
	//output.open("~/ovito/src/gui/actions/bcc.xyz");
	output << std::to_string(atnum)+ " \n";
	output <<"Atoms \n";
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
        output.close();
}
}
