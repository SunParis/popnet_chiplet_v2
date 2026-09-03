#include "router/router.h"

#include <utility>

void CXYRouter::routingAlg(const AddrType& dst, const AddrType& src, long s_ph, long s_vc) {
    this->XY_algorithm(dst, src, s_ph, s_vc);
}

TimeType CXYRouter::getWireDelay(long port) {
    return this->getWireDelay_mesh(port);
}

void CXYRouter::getNextAddress(AddrType& next_addr, long port) {
    this->getNextAddress_mesh(next_addr, port);
}

long CXYRouter::getWirePc(long port) {
    return this->getWirePc_mesh(port);
}

void CXYRouter::getFromRouter(AddrType& from, long port) {
    this->getFromRouter_mesh(from, port);
}

long CXYRouter::getFromPort(long port) {
    return this->getFromPort_mesh(port);
}

CXYRouter::CXYRouter(const Config& config, AddrType addr, RouterList& routers,
                     SimulationState& state, InputTrace& input_trace, DelayWriter& delay_writer)
    : BaseRouter(config, std::move(addr), routers, state, input_trace, delay_writer) {}

void CTXYRouter::routingAlg(const AddrType& dst, const AddrType& src, long s_ph, long s_vc) {
    this->TXY_algorithm(dst, src, s_ph, s_vc);
}

TimeType CTXYRouter::getWireDelay(long port) {
    return this->getWireDelay_mesh(port);
}

void CTXYRouter::getNextAddress(AddrType& next_addr, long port) {
    this->getNextAddress_mesh(next_addr, port);
}

long CTXYRouter::getWirePc(long port) {
    return this->getWirePc_mesh(port);
}

void CTXYRouter::getFromRouter(AddrType& from, long port) {
    this->getFromRouter_mesh(from, port);
}

long CTXYRouter::getFromPort(long port) {
    return this->getFromPort_mesh(port);
}

CTXYRouter::CTXYRouter(const Config& config, AddrType addr, RouterList& routers,
                       SimulationState& state, InputTrace& input_trace, DelayWriter& delay_writer)
    : BaseRouter(config, std::move(addr), routers, state, input_trace, delay_writer) {}

void CChipletMeshRouter::routingAlg(const AddrType& dst, const AddrType& src, long s_ph,
                                    long s_vc) {
    this->chiplet_routing_alg(dst, src, s_ph, s_vc);
}

TimeType CChipletMeshRouter::getWireDelay(long port) {
    return this->getWireDelay_chipletMesh(port);
}

void CChipletMeshRouter::getNextAddress(AddrType& next_addr, long port) {
    this->getNextAddress_chipletMesh(next_addr, port);
}

long CChipletMeshRouter::getWirePc(long port) {
    return this->getWirePc_mesh(port);
}

void CChipletMeshRouter::getFromRouter(AddrType& from, long port) {
    this->getFromRouter_mesh(from, port);
}

long CChipletMeshRouter::getFromPort(long port) {
    return this->getFromPort_mesh(port);
}

CChipletMeshRouter::CChipletMeshRouter(const Config& config, AddrType addr, RouterList& routers,
                                       SimulationState& state, InputTrace& input_trace,
                                       DelayWriter& delay_writer)
    : BaseRouter(config, std::move(addr), routers, state, input_trace, delay_writer) {}

void CChipletStarRouter::routingAlg(const AddrType& dst, const AddrType& src, long s_ph,
                                    long s_vc) {
    this->chiplet_star_topo_routing_alg(dst, src, s_ph, s_vc);
}

TimeType CChipletStarRouter::getWireDelay(long port) {
    return this->getWireDelay_chipletStar(port);
}

void CChipletStarRouter::getNextAddress(AddrType& next_addr, long port) {
    this->getNextAddress_chipletStar(next_addr, port);
}

long CChipletStarRouter::getWirePc(long port) {
    return this->getWirePc_chipletStar(port);
}

void CChipletStarRouter::getFromRouter(AddrType& from, long port) {
    this->getFromRouter_chipletStar(from, port);
}

long CChipletStarRouter::getFromPort(long port) {
    return this->getFromPort_chipletStar(port);
}

CChipletStarRouter::CChipletStarRouter(const Config& config, AddrType addr, RouterList& routers,
                                       SimulationState& state, InputTrace& input_trace,
                                       DelayWriter& delay_writer)
    : BaseRouter(config, std::move(addr), routers, state, input_trace, delay_writer) {}
