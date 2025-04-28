# include "global_defines/defines.h"
# include "global_defines/RGen.h"


/**
 * @brief `operator<<` overload for `MessType`.
 */
std::ostream& operator<<(std::ostream& os, const MessType& MessType_) {
    switch (MessType_) {
        case MessType::EVG:
            os << "EVG";
            break;
        case MessType::ROUTER:
            os << "ROUTER";
            break;
        case MessType::WIRE:
            os << "WIRE";
            break;
        case MessType::CREDIT:
            os << "CREDIT";
            break;
        case MessType::RECONFIGURATION:
            os << "RECONFIGURATION";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}

/**
 * @brief `operator<<` overload for `RoutingType`.
 */
std::ostream& operator<<(std::ostream& os, const RoutingType& RoutingType_) {
    switch (RoutingType_) {
        case RoutingType::XY:
            os << "XY";
            break;
        case RoutingType::TXY:
            os << "TXY";
            break;
        case RoutingType::CHIPLET_ROUTING_MESH:
            os << "CHIPLET_ROUTING_MESH";
            break;
        case RoutingType::CHIPLET_STAR_TOPO_ROUTING:
            os << "CHIPLET_STAR_TOPO_ROUTING";
            break;
        case RoutingType::GRAPH_TOPO:
            os << "GRAPH_TOPO";
            break;
        case RoutingType::RECONFIGURABLE_GRAPH_TOPO:
            os << "RECONFIGURABLE_GRAPH_TOPO";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}

/**
 * @brief `operator<<` overload for `VCStateType`.
 */
std::ostream& operator<<(std::ostream& os, const VCStateType& VCStateType_) {
    switch (VCStateType_) {
        case VCStateType::INIT:
            os << "INIT";
            break;
        case VCStateType::ROUTING:
            os << "ROUTING";
            break;
        case VCStateType::VC_AB:
            os << "VC_AB";
            break;
        case VCStateType::SW_AB:
            os << "SW_AB";
            break;
        case VCStateType::SW_TR:
            os << "SW_TR";
            break;
        case VCStateType::HOME:
            os << "HOME";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}

/**
 * @brief `operator<<` overload for `FlitType`.
 */
std::ostream& operator<<(std::ostream& os, const FlitType& FlitType_) {
    switch (FlitType_) {
        case FlitType::HEADER:
            os << "HEADER";
            break;
        case FlitType::BODY:
            os << "BODY";
            break;
        case FlitType::TAIL:
            os << "TAIL";
            break;
        case FlitType::SINGLE:
            os << "SINGLE";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}

/**
 * @brief `operator<<` overload for `VCShareType`.
 */
std::ostream& operator<<(std::ostream& os, const VCShareType& VCShareType_) {
    switch (VCShareType_) {
        case VCShareType::SHARE:
            os << "SHARE";
            break;
        case VCShareType::MONO:
            os << "MONO";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}

/**
 * @brief `operator<<` overload for `VCUsageType`.
 */
std::ostream& operator<<(std::ostream& os, const VCUsageType& VCUsageType_) {
    switch (VCUsageType_) {
        case VCUsageType::USED:
            os << "USED";
            break;
        case VCUsageType::FREE:
            os << "FREE";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}

/**
 * @brief `operator<<` overload for `SyncProtocolDesc`.
 */
std::ostream& operator<<(std::ostream& os, const SyncProtocolDesc& spd) {
    switch (spd) {
        case SyncProtocolDesc::SPD_ACK:
            os << "SPD_ACK";
            break;
        case SyncProtocolDesc::SPD_PRE_SYNC:
            os << "SPD_PRE_SYNC";
            break;
        case SyncProtocolDesc::SPD_POST_SYNC:
            os << "SPD_POST_SYNC";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}

/**
 * @brief `operator<<` overload for `ProtoState`.
 */
std::ostream& operator<<(std::ostream& os, const ProtoState& state) {
    switch (state) {
        case ProtoState::IDLE:
            os << "IDLE";
            break;
        case ProtoState::PRE_SYNC:
            os << "PRE_SYNC";
            break;
        case ProtoState::POST_SYNC:
            os << "POST_SYNC";
            break;
        case ProtoState::DONE:
            os << "DONE";
            break;
        case ProtoState::DATA_TRANS:
            os << "DATA_TRANS";
            break;
        case ProtoState::ACK_TRANS:
            os << "ACK_TRANS";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}

/**
 * @brief `operator<<` overload for `AddrType`.
 */
std::ostream& operator<<(std::ostream& os, const AddrType& address) {
    os << "(";
    for (long i = 0; i < address.size(); i++) {
        os << address[i];
        if (i != address.size() - 1) {
            os << ", ";
        }
    }
    if (address.size() == 0) {
        os << "N/A";
    }
    os << ")";
    return os;
}