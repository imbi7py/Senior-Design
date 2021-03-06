import ovito
from ovito.io import (import_file, export_file)
from ovito.vis import *
from ovito.data import PipelineFlowState
from ovito.pipeline import StaticSource
import os
import os.path


test_data_dir = "../../files/"

node1 = import_file(test_data_dir + "LAMMPS/class2.data", atom_style = "full")
export_file(node1, "_export_file_test.data", "lammps/data", atom_style = "full")
export_file(node1, "_export_file_test.data", "lammps_data", atom_style = "bond")
export_file(node1, "_export_file_test.data", "lammps/dump", columns = ["Particle Identifier", "Particle Type", "Position.X", "Position.Y", "Position.Z"])
export_file(node1, "_export_file_test.data", "fhi-aims")
export_file(node1, "_export_file_test.data", "imd")
export_file(node1, "_export_file_test.data", "vasp")
export_file(node1, "_export_file_test.data", "povray")
export_file(node1, "_export_file_test.data", "netcdf/amber", columns = ["Particle Identifier", "Particle Type", "Position.X", "Position.Y", "Position.Z"])
export_file(node1, "_export_file_test.data", "xyz", columns = ["Position.X", "Position.Y", "Position.Z"])
os.remove("_export_file_test.data")
ovito.dataset.anim.last_frame = 7
export_file(node1, "_export_file_test.dump", "lammps/dump", columns = ["Position.X", "Position.Y", "Position.Z"], frame = 5)
os.remove("_export_file_test.dump")
export_file(node1, "_export_file_test.dump", "lammps_dump", columns = ["Position.X", "Position.Y", "Position.Z"], multiple_frames = True)
os.remove("_export_file_test.dump")
export_file(node1, "_export_file_test.*.dump", "lammps/dump", columns = ["Position.X", "Position.Y", "Position.Z"], multiple_frames = True, start_frame = 1, end_frame = 5, every_nth_frame = 2)
os.remove("_export_file_test.1.dump")
os.remove("_export_file_test.3.dump")
os.remove("_export_file_test.5.dump")
for i in range(ovito.dataset.anim.last_frame + 1):
    export_file(node1, "_export_file_test.%i.dump" % i, "lammps/dump", columns = ["Position.X", "Position.Y", "Position.Z"], frame = i)
    os.remove("_export_file_test.%i.dump" % i)

# Export a FileSource:
export_file(node1.source, "_export_file_test.dump", "lammps/dump", columns = ["Position.X", "Position.Y", "Position.Z"])
os.remove("_export_file_test.dump")

# Export the entire scene:
export_file(None, "_export_file_test.pov", "povray")
os.remove("_export_file_test.pov")

# Export a PipelineFlowState:
export_file(node1.compute(1), "_export_file_test.dump", "lammps/dump", columns = ["Position.X", "Position.Y", "Position.Z"])
os.remove("_export_file_test.dump")

# Export a StaticSource:
source = StaticSource()
source.particle_properties.create('Position', data=[[0,0,0]])
export_file(source, "_export_file_test.xyz", "xyz", columns = ["Position.X", "Position.Y", "Position.Z"])
os.remove("_export_file_test.xyz")
