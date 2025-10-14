#ifndef CondFormats_Phase2TrackerDTC_TrackerDetToDTCELinkCablingMap_h
#define CondFormats_Phase2TrackerDTC_TrackerDetToDTCELinkCablingMap_h

// -*- C++ -*-
//
// Package:    CondFormats/Phase2TrackerDTC
// Class:      TrackerDetToDTCELinkCablingMap
//
/**\class TrackerDetToDTCELinkCablingMap TrackerDetToDTCELinkCablingMap.cc CondFormats/Phase2TrackerDTC/src/TrackerDetToDTCELinkCablingMap.cc

Description: Map associating DTCELinkId of Phase2 tracker DTCs to DetId of the sensors connected to each of them.

Implementation:
		[Notes on implementation]
*/
//
// Original Author:  Luigi Calligaris, SPRACE, Sao Paulo, BR
// Created        :  Wed, 27 Feb 2019 21:41:13 GMT
//
//

#include <vector>
#include <unordered_map>
#include <cstdint>

#include "CondFormats/Serialization/interface/Serializable.h"
#include "CondFormats/SiPhase2TrackerObjects/interface/DTCELinkId.h"

class TrackerDetToDTCELinkCablingMap {
public:
  TrackerDetToDTCELinkCablingMap();
  virtual ~TrackerDetToDTCELinkCablingMap();

  //enum class Subdet { PXB, FPIX_1, FPIX_2 };

  //using DetObject = std::tuple<unsigned int, unsigned int, Subdet, bool, bool>;

  struct DetObject {
    unsigned int layer;
    unsigned int ring;
    enum class Subdet { PXB, FPIX_1, FPIX_2 };
    Subdet subdet;
    bool zPlus;
    bool xPlus;

    //template <class Archive>
    //void serialize(Archive& ar, const unsigned int /* version */) {
    //  ar & BOOST_SERIALIZATION_NVP(layer);
    //  ar & BOOST_SERIALIZATION_NVP(ring);
    //  ar & BOOST_SERIALIZATION_NVP(subdet);
    //  ar & BOOST_SERIALIZATION_NVP(zPlus);
    //  ar & BOOST_SERIALIZATION_NVP(xPlus);
    //}
  };

  const DetObject& getDetObject(uint32_t const key) const;

  /// Resolves the raw DetId of the detector connected to the eLink identified by a DTCELinkId
  std::unordered_map<DTCELinkId, uint32_t>::const_iterator dtcELinkIdToDetId(DTCELinkId const&) const;

  /// Resolves one or more DTCELinkId of eLinks which are connected to the detector identified by the given raw DetId
  std::pair<std::unordered_multimap<uint32_t, DTCELinkId>::const_iterator,
            std::unordered_multimap<uint32_t, DTCELinkId>::const_iterator>
  detIdToDTCELinkId(uint32_t const) const;

  /// Resolves the layer number associated with the detector identified by the given raw DetId
  unsigned int detIdToLayerNum(uint32_t const key) const;

  /// Resolves the ring number associated with the detector identified by the given raw DetId
  unsigned int detIdToRingNum(uint32_t const key) const;

  /// Resolves the subdetector associated with the detector identified by the given raw DetId
  DetObject::Subdet detIdToSubDet(uint32_t const key) const;
  
  /// Returns true if the detector identified by the given raw DetId is on the plus Z side
  bool detIdToZPlus(uint32_t const key) const;

  /// Returns true if the detector identified by the given raw DetId is on the plus X side
  bool detIdToXPlus(uint32_t const key) const;

  /// Returns true if the cabling map has a record corresponding to a detector identified by the given raw DetId
  bool knowsDTCELinkId(DTCELinkId const&) const;

  /// Returns true if the cabling map has a record corresponding to an eLink identified by the given DTCELinkId
  bool knowsDetId(uint32_t) const;

  /// Returns true if the cabling map has a record corresponding to a layer number identified by the given layerNum
  //bool knowsLayerNum(unsigned int key) const;

  /// Returns true if the cabling map has a record corresponding to a ring number identified by the given ringNum
  //bool knowsRingNum(unsigned int key) const;

  /// Return all DetIds associated with a given DTCId
  std::vector<uint32_t> getAllDetIdsForDTCId(unsigned int dtcId) const;

  // IMPORTANT: The following information is not stored, to preserve space in memory.
  // As these vectors are generated each time the functions are called, you are encouraged to
  // either cache the results or avoid calling them in hot loops.
  // NOTE: This vectors are unsorted

  /// Returns a vector containing all elink DTCELinkId known to the map
  std::vector<DTCELinkId> getKnownDTCELinkIds() const;

  /// Returns a vector containing all DTCIds (unique) known to the map
  std::vector<unsigned int> getKnownDTCIds() const;
  std::vector<std::pair<unsigned int, unsigned int>> getKnownDTCIdsWithIndex() const;

  /// Returns a vector containing all detector DetId known to the map
  std::vector<uint32_t> getKnownDetIds() const;

  /// Inserts in the cabling map a record corresponding to the connection of an eLink identified by the given DTCELinkId to a detector identified by the given raw DetId
  void insert(DTCELinkId const&, uint32_t const, unsigned int const, unsigned int const, DetObject::Subdet const, bool const, bool const);

  /// Clears the map
  void clear();

private:
  std::unordered_multimap<uint32_t, DTCELinkId> cablingMapDetIdToDTCELinkId_;
  std::unordered_map<DTCELinkId, uint32_t> cablingMapDTCELinkIdToDetId_;
  std::unordered_map<uint32_t, DetObject> cablingMapDetIdToDetObject_;
  //std::unordered_map<uint32_t, unsigned int> cablingMapDetIdToRingNum_;

  COND_SERIALIZABLE;
};

//namespace boost {
//namespace serialization {
//  template<class Archive>
//  void serialize(Archive& ar, TrackerDetToDTCELinkCablingMap::DetObject::Subdet& subdet, const unsigned int) {
//    int val = static_cast<int>(subdet);
//    ar & BOOST_SERIALIZATION_NVP(val);
//    subdet = static_cast<TrackerDetToDTCELinkCablingMap::DetObject::Subdet>(val);
//  }
//} // namespace serialization
//}

#endif  // end CondFormats_Phase2TrackerDTC_TrackerDetToDTCELinkCablingMap_h
