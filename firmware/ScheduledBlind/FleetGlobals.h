#pragma once
#include "FleetStorage.h"
#include "core/PeerProtocol.h"
#include "core/FleetDispatch.h"
#include <memory>
struct PeerStatus {uint32_t seen=0,epoch=0;std::string boot;bool contacted=false,positionKnown=false,updating=false,atUpper=false,atLower=false;int percent=0,direction=0;std::string state="Offline",fault,version;};
FleetStorage fleetStorage;std::unique_ptr<FleetRegistry> fleetRegistry;std::unique_ptr<PeerProtocol> peerProtocol;FleetDispatch fleetDispatch;
SemaphoreHandle_t fleetMutex;std::map<std::string,PeerStatus> fleetStatuses;
#include "core/SenderSession.h"
std::map<std::string,SenderSession> senderSessions;
