#include "CondFormats/SiPhase2TrackerObjects/interface/TrackerDetToDTCELinkCablingMap.h"
#include "FWCore/Utilities/interface/Exception.h"

#include <utility>
#include <algorithm>
#include <iostream>

TrackerDetToDTCELinkCablingMap::TrackerDetToDTCELinkCablingMap() {}

TrackerDetToDTCELinkCablingMap::~TrackerDetToDTCELinkCablingMap() {}

std::unordered_map<DTCELinkId, uint32_t>::const_iterator TrackerDetToDTCELinkCablingMap::dtcELinkIdToDetId(
    DTCELinkId const& key) const {
  if (cablingMapDTCELinkIdToDetId_.find(key) == cablingMapDTCELinkIdToDetId_.end()) {
    throw cms::Exception(
        "TrackerDetToDTCELinkCablingMap has been asked to return a DetId associated to a DTCELinkId, but the latter is "
        "unknown to the map. ")
        << " (DTC, GBT, Elink) numbers = (" << key.dtc_id() << "," << key.gbtlink_id() << "," << key.elink_id() << ")"
        << std::endl;
  }

  return cablingMapDTCELinkIdToDetId_.find(key);
}

std::pair<std::unordered_multimap<uint32_t, DTCELinkId>::const_iterator,
          std::unordered_multimap<uint32_t, DTCELinkId>::const_iterator>
TrackerDetToDTCELinkCablingMap::detIdToDTCELinkId(uint32_t const key) const {
  auto const DTCELinkId_itpair = cablingMapDetIdToDTCELinkId_.equal_range(key);

  if (DTCELinkId_itpair.first == cablingMapDetIdToDTCELinkId_.end()) {
    throw cms::Exception(
        "TrackerDetToDTCELinkCablingMap has been asked to return a DTCELinkId associated to a DetId, but the latter is "
        "unknown to the map. ")
        << " DetId = " << key << std::endl;
  }

  return DTCELinkId_itpair;
}

unsigned int TrackerDetToDTCELinkCablingMap::detIdToLayerNum(uint32_t const key) const {
  auto const it = cablingMapDetIdToLayerNum_.find(key);
  
  if (it == cablingMapDetIdToLayerNum_.end()) {
    throw cms::Exception(
        "TrackerDetToDTCELinkCablingMap has been asked to return a layerNum associated to a DetId, but the latter is "
        "unknown to the map. ")
        << " DetId = " << key << std::endl;
  }

  return it->second;
}

unsigned int TrackerDetToDTCELinkCablingMap::detIdToRingNum(uint32_t const key) const {
  auto const it = cablingMapDetIdToRingNum_.find(key);
  
  if (it == cablingMapDetIdToRingNum_.end()) {
    throw cms::Exception(
        "TrackerDetToDTCELinkCablingMap has been asked to return a ringNum associated to a DetId, but the latter is "
        "unknown to the map. ")
        << " DetId = " << key << std::endl;
  }

  return it->second;
}

TrackerDetToDTCELinkCablingMap::Subdet TrackerDetToDTCELinkCablingMap::detIdToSubDet(uint32_t const key) const {
  auto const it = cablingMapDetIdToSubDet_.find(key);

  if (it == cablingMapDetIdToSubDet_.end()) {
    throw cms::Exception(
        "TrackerDetToDTCELinkCablingMap has been asked to return a subDet associated to a DetId, but the latter is "
        "unknown to the map. ")
        << " DetId = " << key << std::endl;
  }

  return it->second;
}

bool TrackerDetToDTCELinkCablingMap::knowsDTCELinkId(DTCELinkId const& key) const {
  return cablingMapDTCELinkIdToDetId_.find(key) != cablingMapDTCELinkIdToDetId_.end();
}

bool TrackerDetToDTCELinkCablingMap::knowsDetId(uint32_t key) const {
  return cablingMapDetIdToDTCELinkId_.find(key) != cablingMapDetIdToDTCELinkId_.end();
}

bool TrackerDetToDTCELinkCablingMap::knowsLayerNum(unsigned int key) const {
  return cablingMapDetIdToLayerNum_.find(key) != cablingMapDetIdToLayerNum_.end();
}

bool TrackerDetToDTCELinkCablingMap::knowsRingNum(unsigned int key) const {
  return cablingMapDetIdToRingNum_.find(key) != cablingMapDetIdToRingNum_.end();
}

std::vector<DTCELinkId> TrackerDetToDTCELinkCablingMap::getKnownDTCELinkIds() const {
  std::vector<DTCELinkId> knownDTCELinkIds(cablingMapDTCELinkIdToDetId_.size());

  // Unzip the map into a vector of DTCELinkId, discarding the DetIds
  std::transform(cablingMapDTCELinkIdToDetId_.begin(),
                 cablingMapDTCELinkIdToDetId_.end(),
                 knownDTCELinkIds.begin(),
                 [=](auto pair) { return pair.first; });

  return knownDTCELinkIds;
}

std::vector<uint32_t> TrackerDetToDTCELinkCablingMap::getKnownDetIds() const {
  std::vector<uint32_t> knownDetId;

  // To get the list of unique DetIds we need to iterate over the various equal_ranges
  // in the map associated to each unique key, and count them only once.

  for (auto allpairs_it = cablingMapDetIdToDTCELinkId_.begin(), allpairs_end = cablingMapDetIdToDTCELinkId_.end();
       allpairs_it != allpairs_end;) {
    // ***Store the first instance of the key***
    knownDetId.push_back(uint32_t(allpairs_it->first));

    // *** Skip to the end of the equal range ***
    // The following is just explicative, the bottom expression is equivalent
    //auto const current_key             = allpairs_it->first;
    //auto const current_key_equal_range = cablingMapDetIdToDTCELinkId_.equal_range(current_key);
    //auto const current_key_range_end   = current_key_equal_range.second;
    auto const current_key_range_end = cablingMapDetIdToDTCELinkId_.equal_range(allpairs_it->first).second;

    while (allpairs_it != current_key_range_end)
      ++allpairs_it;
  }

  return knownDetId;
}

std::vector<uint32_t> TrackerDetToDTCELinkCablingMap::getAllDetIdsForDTCId(unsigned int dtcId) const {
  std::vector<uint32_t> result;
  // Iterate over all (DTCELinkId -> DetId) pairs
  for (auto const& entry : cablingMapDTCELinkIdToDetId_) {
    if (entry.first.dtc_id() == dtcId) {
      result.push_back(entry.second);
    }
  }

  return result;
}

std::vector<unsigned int> TrackerDetToDTCELinkCablingMap::getKnownDTCIds() const {
  std::set<unsigned int> uniqueDTCIds;

  for (const auto& entry : cablingMapDTCELinkIdToDetId_) {
    uniqueDTCIds.insert(entry.first.dtc_id());
  }

  std::vector<unsigned int> dtcIds(uniqueDTCIds.begin(), uniqueDTCIds.end());

  return dtcIds;
}

std::vector<std::pair<unsigned int, unsigned int>> TrackerDetToDTCELinkCablingMap::getKnownDTCIdsWithIndex() const {
  std::set<unsigned int> uniqueDTCIds;
  for (const auto& entry : cablingMapDTCELinkIdToDetId_) {
    uniqueDTCIds.insert(entry.first.dtc_id());
  }

  std::vector<std::pair<unsigned int, unsigned int>> dtcIdsWithIndex;
  dtcIdsWithIndex.reserve(uniqueDTCIds.size());

  unsigned int idx = 0;
  for (auto dtcId : uniqueDTCIds) {
    dtcIdsWithIndex.emplace_back(idx++, dtcId);
  }

  return dtcIdsWithIndex;
}

void TrackerDetToDTCELinkCablingMap::insert(DTCELinkId const& dtcELinkId, uint32_t const detId, unsigned int const layerNum, unsigned int const ringNum, Subdet subDet) {
  cablingMapDTCELinkIdToDetId_.insert(std::make_pair(DTCELinkId(dtcELinkId), uint32_t(detId)));
  cablingMapDetIdToDTCELinkId_.insert(std::make_pair(uint32_t(detId), DTCELinkId(dtcELinkId)));
  cablingMapDetIdToLayerNum_.insert(std::make_pair(detId, layerNum));		// No need to recast these anyway
  cablingMapDetIdToRingNum_.insert(std::make_pair(detId, ringNum));
  cablingMapDetIdToSubDet_.insert(std::make_pair(detId, subDet));
}

void TrackerDetToDTCELinkCablingMap::clear() {
  cablingMapDTCELinkIdToDetId_.clear();
  cablingMapDetIdToDTCELinkId_.clear();
  cablingMapDetIdToLayerNum_.clear();
  cablingMapDetIdToRingNum_.clear();
  cablingMapDetIdToSubDet_.clear();
}
