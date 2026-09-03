#include "router/topo_router.h"

void TopoInfo::readTopo(const std::string& topo_file_path) {
    Sassert(readGvGraph(topo_file_path, this->topo0), "Read topo failed.");
    GraphLib::resetIndex(this->topo0);
}

void TopoInfo::setPipelineDelay() {
    auto nodes = boost::vertices(this->topo0);
    auto pipelineStageDelay = boost::get(&GraphLib::vertex_info::pipelineStageDelay, this->topo0);
    std::for_each(nodes.first, nodes.second, [&](const GraphLib::vertex_t& v) {
        this->routingPeriods.insert(pipelineStageDelay[v]);
    });
}

void TopoInfo::setPortCnt() {
    this->vPortCount.resize(vertexCnt);
    auto nodes = boost::vertices(this->topo0);
    auto name = boost::get(&GraphLib::vertex_p::name, this->topo0);
    std::for_each(nodes.first, nodes.second, [&](const GraphLib::vertex_t& v) {
        TAddressNumber add = std::stol(name[v]);
        this->vPortCount[add] = boost::degree(v, this->topo0) + 1;
    });
}

void TopoInfo::otherInit() {
    this->vertexCnt = boost::num_vertices(this->topo0);
}

void TopoInfo::setTopoVertices() {
    this->topoVertices = std::make_unique<GraphLib::vertex_t[]>(vertexCnt);
    auto nodes = boost::vertices(this->topo0);
    auto name = boost::get(&GraphLib::vertex_info::name, this->topo0);
    std::for_each(nodes.first, nodes.second, [&](const GraphLib::vertex_t& v) {
        TAddressNumber add = std::stol(name[v]);
        this->topoVertices[add] = v;
    });
}

void TopoInfo::calRoutingTable() {
    ShortestPath::TMatrix index_delayTable;
    ShortestPath::TMatrix index_energyTable;
    ShortestPath::TIntMatrix index_routingTable;
    ShortestPath::calculateShortestPathTables(this->topo0, index_delayTable, index_energyTable,
                                              index_routingTable);

    auto name = boost::get(&GraphLib::vertex_info::name, this->topo0);
    auto index = boost::get(&GraphLib::vertex_info::index, this->topo0);
    auto nodes = boost::vertices(this->topo0);
    std::vector<TAddressNumber> index2add(this->vertexCnt);
    std::for_each(nodes.first, nodes.second, [&](const GraphLib::vertex_t& v) {
        TAddressNumber add = std::stol(name[v]);
        index2add[index[v]] = add;
    });

    ShortestPath::resetMatrix(this->energyTable, this->vertexCnt, -1);
    ShortestPath::resetMatrix(this->delayTable, this->vertexCnt, -1);
    ShortestPath::resetIntMatrix(this->routingTable, this->vertexCnt, -1);

    for (TAddressNumber i = 0; i < this->vertexCnt; i++) {
        this->routingTable[i][i] = i;
    }

    for (TAddressNumber i = 0; i < this->vertexCnt; i++) {
        for (TAddressNumber j = 0; j < this->vertexCnt; j++) {
            TAddressNumber add1 = index2add[i];
            TAddressNumber add2 = index2add[j];
            this->delayTable[add1][add2] = index_delayTable[i][j];
            this->energyTable[add1][add2] = index_energyTable[i][j];
            this->routingTable[add1][add2] = index2add[index_routingTable[i][j]];
        }
    }

    this->portMap = std::make_unique<std::vector<SLinkInfo>[]>(this->vertexCnt);
    this->nextHop_port_map =
        std::make_unique<std::unordered_map<TAddressNumber, TAddressNumber>[]>(this->vertexCnt);
    for (TAddressNumber i = 0; i < this->vertexCnt; i++) {
        this->nextHop_port_map[i][i] = 0;
        this->portMap[i].push_back({0, 0, i});
    }

    auto edgeList = boost::edges(this->topo0);
    auto delay = boost::get(boost::edge_weight_t::edge_weight, this->topo0);
    std::for_each(edgeList.first, edgeList.second, [&](const GraphLib::edge_t& e) {
        GraphLib::vertex_t u = boost::source(e, this->topo0);
        GraphLib::vertex_t v = boost::target(e, this->topo0);
        TAddressNumber add1 = index2add[index[u]];
        TAddressNumber add2 = index2add[index[v]];
        TimeType d = delay[e];
        TAddressNumber p = this->portMap[add1].size();
        TAddressNumber q = this->portMap[add2].size();
        this->portMap[add1].push_back({q, d, add2});
        this->portMap[add2].push_back({p, d, add1});
        this->nextHop_port_map[add1][add2] = p;
        this->nextHop_port_map[add2][add1] = q;
    });
}

TopoInfo::TopoInfo(const std::string& topo_file_path) {
    this->readTopo(topo_file_path);
    this->otherInit();
    this->setPipelineDelay();
    this->setPortCnt();
    this->setTopoVertices();
    this->calRoutingTable();
}

void CGraphTopo::routingAlg(const AddrType& dst, const AddrType&, long s_ph, long s_vc) {
    TAddressNumber nextAdd = this->topo_info.routingTable[this->address_.front()][dst.front()];
    auto it = this->topo_info.nextHop_port_map[this->address_.front()].find(nextAdd);
    if (it == this->topo_info.nextHop_port_map[this->address_.front()].end()) {
        Logger::error("Routing error: dst {} in {} next: {}", dst.front(), this->address_.front(),
                      nextAdd);
        Sassert(false, "Routing error.");
    }
    TAddressNumber port = it->second;
    for (long i = 0; i < this->config_.getVirtualChannelNumber(); ++i) {
        this->input_module_.addRouting(s_ph, s_vc, VCType(port, i));
    }
}

TimeType CGraphTopo::getWireDelay(long port) {
    return this->topo_info.portMap[this->address_.front()][port].linkDelay;
}

void CGraphTopo::getNextAddress(AddrType& next_addr, long port) {
    next_addr.clear();
    next_addr.push_back(this->topo_info.portMap[this->address_.front()][port].neighbour);
}

long CGraphTopo::getWirePc(long port) {
    return this->topo_info.portMap[this->address_.front()][port].neighbourPort;
}

void CGraphTopo::getFromRouter(AddrType& from, long port) {
    from.clear();
    from.push_back(this->topo_info.portMap[this->address_.front()][port].neighbour);
}

long CGraphTopo::getFromPort(long port) {
    return this->topo_info.portMap[this->address_.front()][port].neighbourPort;
}

TimeType CGraphTopo::getPipeStageDelay() const {
    return this->topo_info.topo0[this->topo_info.topoVertices[this->address_.front()]]
        .pipelineStageDelay;
}

CGraphTopo::CGraphTopo(const Config& config, AddrType addr, RouterList& routers, TopoInfo& topology,
                       SimulationState& state, InputTrace& input_trace, DelayWriter& delay_writer)
    : BaseRouter(config, addr, routers, state, input_trace, delay_writer,
                 static_cast<long>(topology.vPortCount.at(static_cast<std::size_t>(addr.front())))),
      topo_info(topology) {}

void ReconfigTopoInfo::readTopo(const std::string& topo_file_path) {
    Sassert(readGvGraph(topo_file_path, this->topo0), "Read topo failed.");
    GraphLib::resetIndex(this->topo0);
}

void ReconfigTopoInfo::setPipelineDelay(GraphLib::graph_t& topo) {
    auto nodes = boost::vertices(topo);
    auto pipelineStageDelay = boost::get(&GraphLib::vertex_info::pipelineStageDelay, topo);
    std::for_each(nodes.first, nodes.second, [&](const GraphLib::vertex_t& v) {
        this->routingPeriods.insert(pipelineStageDelay[v]);
    });
}

void ReconfigTopoInfo::setPortCnt() {
    this->vPortCount.resize(vertexCnt);
    auto nodes = boost::vertices(this->topo0);
    auto name = boost::get(&GraphLib::vertex_p::name, this->topo0);
    std::for_each(nodes.first, nodes.second, [&](const GraphLib::vertex_t& v) {
        TAddressNumber add = std::stol(name[v]);
        this->vPortCount[add] = boost::degree(v, this->topo0) + 1;
    });
}

void ReconfigTopoInfo::otherInit() {
    this->vertexCnt = boost::num_vertices(this->topo0);
}

void ReconfigTopoInfo::setTopoVertices(GraphLib::graph_t& topo) {
    this->topoVertices = std::make_unique<GraphLib::vertex_t[]>(vertexCnt);
    auto nodes = boost::vertices(topo);
    auto name = boost::get(&GraphLib::vertex_info::name, topo);
    std::for_each(nodes.first, nodes.second, [&](const GraphLib::vertex_t& v) {
        TAddressNumber add = std::stol(name[v]);
        this->topoVertices[add] = v;
    });
}

void ReconfigTopoInfo::calRoutingTable(GraphLib::graph_t& topo) {
    ShortestPath::TMatrix index_delayTable;
    ShortestPath::TMatrix index_energyTable;
    ShortestPath::TIntMatrix index_routingTable;
    ShortestPath::calculateShortestPathTables(topo, index_delayTable, index_energyTable,
                                              index_routingTable);

    auto name = boost::get(&GraphLib::vertex_info::name, topo);
    auto index = boost::get(&GraphLib::vertex_info::index, topo);
    auto nodes = boost::vertices(topo);
    std::vector<TAddressNumber> index2add(this->vertexCnt);
    std::for_each(nodes.first, nodes.second, [&](const GraphLib::vertex_t& v) {
        TAddressNumber add = std::stol(name[v]);
        index2add[index[v]] = add;
    });

    ShortestPath::resetMatrix(this->energyTable, this->vertexCnt, -1);
    ShortestPath::resetMatrix(this->delayTable, this->vertexCnt, -1);
    ShortestPath::resetIntMatrix(this->routingTable, this->vertexCnt, -1);

    for (TAddressNumber i = 0; i < this->vertexCnt; i++) {
        this->routingTable[i][i] = i;
    }

    for (TAddressNumber i = 0; i < this->vertexCnt; i++) {
        for (TAddressNumber j = 0; j < this->vertexCnt; j++) {
            TAddressNumber add1 = index2add[i];
            TAddressNumber add2 = index2add[j];
            this->delayTable[add1][add2] = index_delayTable[i][j];
            this->energyTable[add1][add2] = index_energyTable[i][j];
            this->routingTable[add1][add2] = index2add[index_routingTable[i][j]];
        }
    }

    this->portMap = std::make_unique<std::vector<SLinkInfo>[]>(this->vertexCnt);
    this->nextHop_port_map =
        std::make_unique<std::unordered_map<TAddressNumber, TAddressNumber>[]>(this->vertexCnt);
    for (TAddressNumber i = 0; i < this->vertexCnt; i++) {
        this->nextHop_port_map[i][i] = 0;
        this->portMap[i].push_back({0, 0, i});
    }

    auto edgeList = boost::edges(topo);
    auto delay = boost::get(boost::edge_weight_t::edge_weight, topo);
    std::for_each(edgeList.first, edgeList.second, [&](const GraphLib::edge_t& e) {
        GraphLib::vertex_t u = boost::source(e, topo);
        GraphLib::vertex_t v = boost::target(e, topo);
        TAddressNumber add1 = index2add[index[u]];
        TAddressNumber add2 = index2add[index[v]];
        TimeType d = delay[e];
        TAddressNumber p = this->portMap[add1].size();
        TAddressNumber q = this->portMap[add2].size();
        this->portMap[add1].push_back({q, d, add2});
        this->portMap[add2].push_back({p, d, add1});
        this->nextHop_port_map[add1][add2] = p;
        this->nextHop_port_map[add2][add1] = q;
    });
}

void ReconfigTopoInfo::readReconfigurationFile(const std::string& reconfigurationFilePath) {
    std::ifstream ifs(reconfigurationFilePath);
    ifs >> this->periodCnt >> this->reconfigPeriod >> this->reconfigRouterCnt;
    int64_t t;
    this->reconfigRouters.reserve(this->reconfigRouterCnt);
    for (std::int64_t i = 0; i < this->reconfigRouterCnt; i++) {
        ifs >> t;
        this->reconfigRouters.push_back(t);
    }
    this->flows.reserve(this->periodCnt);
    SFlow ft;
    for (std::int64_t i = 0; i < this->periodCnt; i++) {
        this->flows.emplace_back();
        ifs >> t;
        this->flows.back().reserve(t);
        for (std::int64_t j = 0; j < t; j++) {
            ifs >> ft.src >> ft.dst >> ft.delay;
            this->flows.back().push_back(ft);
        }
    }
}

void ReconfigTopoInfo::setPortCnt_rtr(GraphLib::graph_t& topo) {
    std::unordered_set<TAddressNumber> interposerRouterSet(this->reconfigRouters.begin(),
                                                           this->reconfigRouters.end());
    this->vPortCount.resize(this->vertexCnt);
    auto nodes = boost::vertices(topo);
    auto name = boost::get(&GraphLib::vertex_p::name, topo);
    std::for_each(nodes.first, nodes.second, [&](const GraphLib::vertex_t& v) {
        TAddressNumber add = stol(name[v]);
        this->vPortCount[add] =
            boost::degree(v, topo) + 1 + (interposerRouterSet.count(add) > 0 ? 4 : 0);
    });
}

void ReconfigTopoInfo::reconfigurateTopology(std::int64_t periodNumber) {
    this->resetTopology();
    for (auto& newLink : this->flows[periodNumber]) {
        GraphLib::edge_p ep{newLink.delay};
        boost::add_edge(this->topoVertices[newLink.src], this->topoVertices[newLink.dst], ep,
                        this->curTopo);
    }
}

void ReconfigTopoInfo::resetTopology() {
    this->curTopo = this->topo0;
    this->setTopoVertices(this->curTopo);
}

void ReconfigTopoInfo::swapRoutingInfo() {
    this->old_routingTable.resize(boost::extents[this->vertexCnt][this->vertexCnt]);
    std::copy_n(this->routingTable.data(), this->routingTable.num_elements(),
                this->old_routingTable.data());
    this->old_portMap = std::move(this->portMap);
    this->old_nextHop_port_map = std::move(this->nextHop_port_map);
}

double ReconfigTopoInfo::getReconfigurationPeriod() {
    return this->reconfigPeriod;
}

std::int64_t ReconfigTopoInfo::getCurrentReconfigurationPeriod() {
    return this->curReconfigPeriodNumber;
}

void ReconfigTopoInfo::reconfigurate(TimeType reconfigurationTime,
                                     TimeType next_reconfigurationTime) {
    if (this->curReconfigPeriodNumber < this->periodCnt) {
        this->reconfigurateTopology(this->curReconfigPeriodNumber);
        this->swapRoutingInfo();
        this->calRoutingTable(this->curTopo);
        this->lastReconfigurationTime = reconfigurationTime;
        this->nextReconfigurationTime = next_reconfigurationTime;
        this->curReconfigPeriodNumber += 1;
    }
}

bool ReconfigTopoInfo::hasNextReconfiguration() const noexcept {
    return this->curReconfigPeriodNumber < this->periodCnt;
}

ReconfigTopoInfo::ReconfigTopoInfo(const std::string& topo_file_path,
                                   const std::string& reconfig_file_path) {
    this->readTopo(topo_file_path);
    this->otherInit();
    this->readReconfigurationFile(reconfig_file_path);
    this->curReconfigPeriodNumber = RECONFIGURATION_PERIOD_NUMBER_0;
    this->lastReconfigurationTime = 0;
    this->setPipelineDelay(this->topo0);
    this->setPortCnt_rtr(this->topo0);
    this->resetTopology();
    this->calRoutingTable(this->curTopo);
    this->nextReconfigurationTime = 0;
}

void CReconfigTopoRouter::routingAlg(const AddrType& dst, const AddrType&, long s_ph, long s_vc) {
    TAddressNumber nextAdd = this->topo_info.routingTable[this->address_.front()][dst.front()];
    auto it = this->topo_info.nextHop_port_map[this->address_.front()].find(nextAdd);
    if (it == this->topo_info.nextHop_port_map[this->address_.front()].end()) {
        Logger::error("Routing error: dst {} in {} next: {}", dst.front(), this->address_.front(),
                      nextAdd);
        Sassert(false, "Routing error.");
    }
    TAddressNumber port = it->second;
    for (long i = 0; i < this->config_.getVirtualChannelNumber(); ++i) {
        this->input_module_.addRouting(s_ph, s_vc, VCType(port, i));
    }
}

TimeType CReconfigTopoRouter::getWireDelay(long port) {
    return this->topo_info.portMap[this->address_.front()][port].linkDelay;
}

void CReconfigTopoRouter::getNextAddress(AddrType& next_addr, long port) {
    next_addr.clear();
    next_addr.push_back(this->topo_info.portMap[this->address_.front()][port].neighbour);
}

long CReconfigTopoRouter::getWirePc(long port) {
    return this->topo_info.portMap[this->address_.front()][port].neighbourPort;
}

void CReconfigTopoRouter::getFromRouter(AddrType& from, long port) {
    from.clear();
    from.push_back(this->topo_info.portMap[this->address_.front()][port].neighbour);
}

long CReconfigTopoRouter::getFromPort(long port) {
    return this->topo_info.portMap[this->address_.front()][port].neighbourPort;
}

TimeType CReconfigTopoRouter::getPipeStageDelay() const {
    return this->topo_info.curTopo[this->topo_info.topoVertices[this->address_.front()]]
        .pipelineStageDelay;
}

bool CReconfigTopoRouter::isEventAfterReconfiguration(TimeType delay) {
    return this->state_.currentTime() - delay >= this->topo_info.lastReconfigurationTime;
}

bool CReconfigTopoRouter::isNearReconfiguration() {
    return this->state_.currentTime() + RECONFIGURATION_INTERVAL >=
           this->topo_info.nextReconfigurationTime;
}

CReconfigTopoRouter::CReconfigTopoRouter(const Config& config, AddrType addr, RouterList& routers,
                                         ReconfigTopoInfo& topology, SimulationState& state,
                                         InputTrace& input_trace, DelayWriter& delay_writer)
    : BaseRouter(config, addr, routers, state, input_trace, delay_writer,
                 static_cast<long>(topology.vPortCount.at(static_cast<std::size_t>(addr.front())))),
      topo_info(topology) {}
