## Datapoint abstraction

A datapoint is any source or receiver of data\

it has:
- an ID
- a write method
- a read method
- bookkeeping info (uptime, accessed by threads etc.)
- MMIO location (if necessary)
  - entry in the UML (unified mapping layer)


### Datapoint location and function

Datapoints are stored in Datapoint-registry-entries\
which are in the Datapoint-registry

This is located at a fixed location and has extra frames allocated to it.
This is an implementation of an arena-allocator
The ID is correlated to the datapoint via a hashmap.

### Memory layout (Datapoint-registry)

#### Linear access Hashmap.
starting at PML4 entry 510 (uses an arena allocator style for gathering phys frames)\


#### ID-Heap / Datapoint-registry