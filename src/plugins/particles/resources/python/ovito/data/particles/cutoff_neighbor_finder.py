# Load dependencies
import ovito
from ovito.data import SimulationCell

# Load the native code module
import ovito.plugins.Particles

class CutoffNeighborFinder(ovito.plugins.Particles.CutoffNeighborFinder):
    """ 
    A utility class that computes particle neighbor lists.
    
    This class allows to iterate over the neighbors of each particle within a given cutoff distance.
    You can use it to build neighbors lists or perform other kinds of analyses that require neighbor information.
    
    The constructor takes a positive cutoff radius and a :py:class:`DataCollection <ovito.data.DataCollection>` 
    containing the input particle positions and the cell geometry (including periodic boundary flags).
    
    Once the :py:class:`!CutoffNeighborFinder` has been constructed, you can call its :py:meth:`.find` method to 
    iterate over the neighbors of a specific particle, for example:
    
    .. literalinclude:: ../example_snippets/cutoff_neighbor_finder.py
    
    If you want to determine the *N* nearest neighbors of a particle,
    use the :py:class:`NearestNeighborFinder` class instead.    
    """
        
    def __init__(self, cutoff, data_collection):
        """ This is the constructor. """
        super(self.__class__, self).__init__()
        if 'Position' not in data_collection.particle_properties:
            raise KeyError("DataCollection does not contain any particles.")
        pos_property = data_collection.particle_properties['Position']
        self.particle_count = len(pos_property)
        if not self.prepare(cutoff, pos_property, data_collection.expect(SimulationCell)):
            raise RuntimeError("Operation has been canceled by the user.")
        
    def find(self, index):
        """ 
        Returns an iterator over all neighbors of the given particle.
         
        :param int index: The index of the central particle whose neighbors should be iterated. Particle indices start at 0.
        :returns: A Python iterator that visits all neighbors of the central particle within the cutoff distance. 
                  For each neighbor the iterator returns an object with the following attributes:
                  
                      * **index**: The index of the current neighbor particle (starting at 0).
                      * **distance**: The distance of the current neighbor from the central particle.
                      * **distance_squared**: The squared neighbor distance.
                      * **delta**: The three-dimensional vector connecting the central particle with the current neighbor (taking into account periodicity).
                      * **pbc_shift**: The periodic shift vector, which specifies how often each periodic boundary of the simulation cell is crossed when going from the central particle to the current neighbor.
        
        Note that all periodic images of particles within the cutoff radius are visited. Thus, the same particle index may appear multiple times in the neighbor
        list of a central particle. In fact, the central particle may be among its own neighbors in a sufficiently small periodic simulation cell.
        However, the computed vector (``delta``) and PBC shift (``pbc_shift``) taken together will be unique for each visited image of a neighboring particle.
        """
        if index < 0 or index >= self.particle_count:
            raise IndexError("Particle index is out of range.")
        # Construct the C++ neighbor query. 
        query = ovito.plugins.Particles.CutoffNeighborFinder.Query(self, int(index))
        # Iterate over neighbors.
        while not query.at_end:
            yield query
            query.next()

