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
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>
#include <vector>


namespace Ovito { OVITO_BEGIN_INLINE_NAMESPACE(Gui)
void ActionManager::cut(const QString & name, float xc, float yc, float zc, float radius) {
    std::string fname = name.toStdString();
    std::ifstream ifs(fname);    
    int atoms(0), count(0), type;
    float x, y, z;
    std::vector<float> pos, result;
    std::vector<int> typePos, typeResult;
    std::string line;
    
    ifs >> atoms >> line;
    // skip second line
    
    // find the center of cubic lattice
    for (int i = 0; i < atoms; i++) {
        ifs >> type >> x >> y >> z;
        typePos.push_back(type);
        pos.push_back(x);
        pos.push_back(y);
        pos.push_back(z);
        xc = xc + x;
        yc = yc + y;
        zc = zc + z;
    }
    xc = xc / atoms;
      yc = yc / atoms;
     zc = zc / atoms;
    
    for (int i = 0; i < atoms; i++) {
        float distsq = (pos[3*i] - xc) * (pos[3*i] - xc) + (pos[3*i+1] - yc) * (pos[3*i+1] - yc) + (pos[3*i+2] - zc) * (pos[3*i+2] - zc);
        if (distsq <= radius * radius) {
            count++;
            typeResult.push_back(typePos[i]);
            result.push_back(pos[3*i]);
            result.push_back(pos[3*i+1]);
            result.push_back(pos[3*i+2]);
        }
    }
    
    // generate output datafile
        std::ofstream output;
        // modify the directory if necessary
        output.open(fname);
        output << count << std::endl << "Atoms" << std::endl;
        
        for (int i = 0; i < count; i++) { 
            char buff[100];
            std::snprintf(buff, sizeof(buff), "%d      %E    %E    %E\r\n",
                    typeResult[i], result[3*i], result[3*i+1], result[3*i+2]);
            std::string str = buff;
            output << str;
        }
        output.close();
}
}


