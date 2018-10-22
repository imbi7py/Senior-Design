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

void ActionManager::NaCl(int x, int y, int z, float acell) {
   // create a vector to store coordinates
   std::vector<int> typePos, resultPos;
   std::vector<float> posX, posY, posZ, result;
   // corner
   posX.push_back(0); posY.push_back(0); posZ.push_back(0); typePos.push_back(1);
   // face
   posX.push_back(acell/2); posY.push_back(acell/2); posZ.push_back(0); typePos.push_back(1);
   posX.push_back(0); posY.push_back(acell/2); posZ.push_back(acell/2); typePos.push_back(1);
   posX.push_back(acell/2); posY.push_back(0); posZ.push_back(acell/2); typePos.push_back(1);
   // diamond
   posX.push_back(acell/2); posY.push_back(0); posZ.push_back(0); typePos.push_back(2);
   posX.push_back(0); posY.push_back(acell/2); posZ.push_back(0); typePos.push_back(2);
   posX.push_back(0); posY.push_back(0); posZ.push_back(acell/2); typePos.push_back(2);
   posX.push_back(acell/2); posY.push_back(acell/2); posZ.push_back(acell/2); typePos.push_back(2);

   int atnum = 0;
   for (int i1 = 0; i1 < x; i1++) {
       for (int i2 = 0; i2 < y; i2++) {
           for (int i3 = 0; i3 < z; i3++) {
               for (int i = 0; i < 8; i++) {
                   atnum++;
                   resultPos.push_back(typePos[i]);
                   result.push_back(posX[i] + (i1+1) * acell);
                   result.push_back(posY[i] + (i2+1) * acell);
                   result.push_back(posZ[i] + (i3+1) * acell);
               }
           }
       }
   }
   // generate output datafile
   std::ofstream output;
   output.open("NaCl.xyz");

   output << atnum << std::endl << "Atoms" << std::endl;

   for (int i = 0; i < atnum; i++) {
       int typeatom(0);
       if (i <= atnum) {
           char buff[100];
           std::snprintf(buff, sizeof(buff), "%d      %E    %E    %E\r\n",
                   resultPos[i], result[3*i], result[3*i+1], result[3*i+2]);
           std::string str = buff;
           output << str;
       }
   }
   output.close();
}
}


