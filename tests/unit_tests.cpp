#include "test_support.h"

#include <cmath>
#include <filesystem>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

#include "global_defines/input_trace.h"
#include "global_defines/message_define.h"
#include "global_defines/proto_engine.h"
#include "io/delay_writer.h"
#include "logger/logger.hpp"
#include "preprocess/config.h"
#include "router/modules.h"
#include "router/topo_router.h"
#include "sim/simulation_state.h"

namespace {

const std::filesystem::path fixture_dir{POPNET_TEST_FIXTURES};

void eventQueueIsStableAndPreservesPeriods() {
    MessQueue queue;
    queue.addMessage(MessEvent(2, MessType::WIRE, AddrType{10}, AddrType{11}, 1, 0));
    queue.addMessage(MessEvent(2, MessType::EVG));
    queue.addMessage(MessEvent(2, MessType::ROUTER, 0.5));
    queue.addMessage(MessEvent(2, MessType::RECONFIGURATION));
    queue.addMessage(MessEvent(2, MessType::CREDIT, AddrType{12}, AddrType{13}, 2, 1));

    POPNET_CHECK_EQ(queue.size(), std::size_t{5});
    POPNET_CHECK_EQ(queue.popMessage().getEventType(), MessType::CREDIT);
    POPNET_CHECK_EQ(queue.popMessage().getEventType(), MessType::WIRE);
    POPNET_CHECK_EQ(queue.popMessage().getEventType(), MessType::RECONFIGURATION);
    const auto router = queue.popMessage();
    POPNET_CHECK_EQ(router.getEventType(), MessType::ROUTER);
    POPNET_CHECK_NEAR(router.getRoutingPeriod(), 0.5, 1e-12);
    POPNET_CHECK_EQ(queue.popMessage().getEventType(), MessType::EVG);
    POPNET_CHECK(queue.empty());

    queue.addMessage(MessEvent(0, MessType::ROUTER, 1));
    queue.addMessage(MessEvent(0, MessType::ROUTER, 2.5));
    queue.addMessage(MessEvent(10, MessType::WIRE));
    queue.advanceRouterCycles(9);
    POPNET_CHECK_NEAR(queue.getTop(MessType::ROUTER).getEventStart(), 9, 1e-12);
    queue.popMessage();
    POPNET_CHECK_NEAR(queue.getTop(MessType::ROUTER).getEventStart(), 10, 1e-12);
    POPNET_CHECK_NEAR(queue.getTop(MessType::ROUTER).getRoutingPeriod(), 2.5, 1e-12);

    queue.clear();
    queue.addMessage(MessEvent(10, MessType::EVG));
    queue.updateEVGCycle(5);
    POPNET_CHECK_NEAR(queue.getTop().getEventStart(), 5, 1e-12);
    queue.clear(MessType::EVG);
    POPNET_CHECK(queue.empty());
}

void normalAndProtocolTracesKeepSourceOrdering() {
    ProtocolEngine protocols;
    InputTrace trace((fixture_dir / "normal.trace").string(), false, 1, protocols);
    trace.readTraceFile();

    POPNET_CHECK(trace.isReadFin());
    POPNET_CHECK_EQ(trace.packetCount(), std::size_t{3});
    POPNET_CHECK_NEAR(trace.front().start_time, 1, 1e-12);
    POPNET_CHECK_EQ(trace.front().src_addr, AddrType{1});
    POPNET_CHECK_NEAR(trace.front(AddrType{0}).start_time, 2, 1e-12);
    trace.popFront(AddrType{0});
    POPNET_CHECK_NEAR(trace.front(AddrType{0}).start_time, 3, 1e-12);
    trace.popFront();
    POPNET_CHECK_NEAR(trace.front().start_time, 2, 1e-12);

    ProtocolEngine protocol_trace_engine;
    InputTrace protocol_trace((fixture_dir / "protocol.trace").string(), true, 1,
                              protocol_trace_engine);
    protocol_trace.readTraceFile();
    POPNET_CHECK_EQ(protocol_trace.packetCount(), std::size_t{2});
    POPNET_CHECK_EQ(protocol_trace_engine.size(), std::size_t{2});
    POPNET_CHECK_EQ(protocol_trace.front().des_addr, AddrType{0});
    const auto& transaction = protocol_trace_engine.get(protocol_trace.front().id);
    POPNET_CHECK((transaction.protoDesc & SPD_BARRIER) != 0);
    POPNET_CHECK_EQ(transaction.des_addr, AddrType{1});

    popnet_test::TempDirectory temp("malformed-trace");
    const auto malformed = temp.path() / "malformed.trace";
    popnet_test::writeFile(malformed, "0 0\n");
    ProtocolEngine unused;
    InputTrace malformed_trace(malformed.string(), false, 1, unused);
    POPNET_EXPECT_THROW(std::runtime_error, malformed_trace.readTraceFile());
}

void protocolLookupIsIndexedAndRejectsDuplicates() {
    ProtoPacket packet(1);
    packet.id = 42;
    packet.src_time = 3;
    packet.des_time = 8;
    packet.src_addr = {1};
    packet.des_addr = {2};
    packet.packet_size = 4;
    packet.proto_dsc = SPD_LOCK | 7;

    ProtocolEngine engine;
    const auto generated = engine.add(packet);
    POPNET_CHECK_EQ(generated.id, TId{42});
    POPNET_CHECK_EQ(generated.des_addr, AddrType{0});
    POPNET_CHECK_EQ(engine.get(42).status, ProtoState::DATA_TRANS);
    engine.get(42).status = ProtoState::DONE;
    POPNET_CHECK_EQ(static_cast<const ProtocolEngine&>(engine).get(42).status, ProtoState::DONE);
    POPNET_EXPECT_THROW(std::runtime_error, engine.add(packet));
    POPNET_EXPECT_THROW(std::out_of_range, engine.get(99));
    engine.clear();
    POPNET_CHECK_EQ(engine.size(), std::size_t{0});
}

void flatRouterBuffersPreserveCreditsAndMoves() {
    InputModules input(3, 2);
    input.setState(1, 1, VCStateType::ROUTING);
    input.addRouting(1, 1, VCType{2, 0});
    input.setCRouting(1, 1, VCType{2, 1});

    Flit flit(7, FlitType::SINGLE, AddrType{0}, AddrType{1}, 2, DataType{11, 22}, 99);
    flit.setCreditReturn(AddrType{2}, 1);
    input.addFlit(1, 1, std::move(flit));
    POPNET_CHECK_EQ(input.getBufferSize(1, 1), std::size_t{1});
    POPNET_CHECK_EQ(input.getFlit(1, 1).getFlitID(), TFlitId{7});
    POPNET_CHECK_EQ(input.getFlit(1, 1).getCreditReturnAddress(), AddrType{2});
    POPNET_CHECK_EQ(input.getFlit(1, 1).getCreditReturnPort(), 1L);
    POPNET_CHECK_EQ(input.getState(1, 1), VCStateType::ROUTING);
    POPNET_CHECK_EQ(input.getRouting(1, 1).front(), VCType(2, 0));
    POPNET_CHECK_EQ(input.getCRouting(1, 1), VCType(2, 1));
    input.removeFlit(1, 1);
    POPNET_CHECK_EQ(input.getBufferSize(1, 1), std::size_t{0});
    POPNET_EXPECT_THROW(std::out_of_range, input.getFlit(3, 0));

    OutputModules output(3, 2, 4, 2);
    POPNET_CHECK_EQ(output.getCounter(2, 1), 4L);
    output.decCounter(2, 1);
    output.incCounter(2, 1);
    POPNET_CHECK_EQ(output.getCounter(2, 1), 4L);
    output.acquireChannel(2, 1, VCType{1, 1});
    POPNET_CHECK_EQ(output.getUsage(2, 1), VCUsageType::USED);

    Flit outgoing(8, FlitType::SINGLE, AddrType{0}, AddrType{1}, 2, DataType{33, 44}, 100);
    output.addFlit(2, std::move(outgoing));
    output.addAddr(2, VCType{2, 1});
    POPNET_CHECK_EQ(output.getOutBufferSize(2), std::size_t{1});
    POPNET_CHECK_EQ(output.getLocalCounter(2), 1L);
    POPNET_CHECK_EQ(output.getAddr(2), VCType(2, 1));
    output.removeFlit(2);
    output.removeAddr(2);
    output.releaseChannel(2, 1);
    POPNET_CHECK_EQ(output.getLocalCounter(2), 2L);
    POPNET_CHECK_EQ(output.getUsage(2, 1), VCUsageType::FREE);
}

void powerWrapperUsesBoundedStorage() {
    PowerModules power(3, 2, 2, 1.5);
    const DataType first{0x1234, 0x5678};
    const DataType second{0x9abc, 0xdef0};
    power.addBufferWritePwr(0, first);
    power.addBufferWritePwr(0, second);
    power.addBufferReadPwr(0, second);
    power.addCrossbarTravPwr(0, 1, second);
    power.addVCArbitPwr(1, 0, 1, 1);
    power.addLinkTravPwr(1, second);

    POPNET_CHECK(std::isfinite(power.getBufferPower()));
    POPNET_CHECK(std::isfinite(power.getCrossbarPower()));
    POPNET_CHECK(std::isfinite(power.getArbiterPower()));
    POPNET_CHECK(std::isfinite(power.getLinkPower()));
}

void configurationParsesJsonAndAllFlagKinds() {
    popnet_test::TempDirectory temp("config");
    const auto trace_path = fixture_dir / "normal.trace";
    const auto json_path = temp.path() / "config.json";
    const auto log_path = temp.path() / "json.log";
    const auto delay_path = temp.path() / "json.delay";

    nlohmann::json json{
        {"vertices", 3},
        {"dimension", 1},
        {"vc_cnt", 2},
        {"input_buffer", 4},
        {"output_buffer", 3},
        {"link_length", 9.5},
        {"time", 20.25},
        {"routing_algorithm", "XY"},
        {"trace_file", trace_path.string()},
        {"log_file", log_path.string()},
        {"delay_file", delay_path.string()},
        {"random_seed", 17},
    };
    popnet_test::writeFile(json_path, json.dump());
    popnet_test::Arguments json_args{"test", "-JSON", json_path.string()};
    Config json_config(json_args.argc(), json_args.argv());
    POPNET_CHECK_EQ(json_config.getAryNumber(), 3L);
    POPNET_CHECK_EQ(json_config.getPhysicalPortNumber(), 3L);
    POPNET_CHECK_EQ(json_config.getFlitSize(), 4L);
    POPNET_CHECK_NEAR(json_config.getLinkLength(), 9.5, 1e-12);
    POPNET_CHECK_NEAR(json_config.getSimLength(), 20.25, 1e-12);
    POPNET_CHECK_EQ(json_config.getRandomSeed().value(), 17L);

    popnet_test::CurrentPathGuard current_path(temp.path());
    popnet_test::Arguments cli_args({
        "test", "-A", "2",    "-c",  "2",  "-V", "2",
        "-B",   "4",  "-O",   "3",   "-F", "2",  "-L",
        "1.25", "-T", "10.5", "-r",  "7",  "-I", trace_path.string(),
        "-R",   "1",  "-m",   "2.5", "-l", "-D", (temp.path() / "cli.delay").string(),
        "-P",   "-E",
    });
    Config cli_config(cli_args.argc(), cli_args.argv());
    POPNET_CHECK_NEAR(cli_config.getLinkLength(), 1.25, 1e-12);
    POPNET_CHECK_NEAR(cli_config.getSimLength(), 10.5, 1e-12);
    POPNET_CHECK_EQ(cli_config.getRoutingAlg(), RoutingType::TXY);
    POPNET_CHECK(cli_config.isPacketLoss());
    POPNET_CHECK(cli_config.isSyncProtocolEnable());
    POPNET_CHECK(cli_config.isEndWithMinus1());

    json["vertices"] = 0;
    const auto invalid_path = temp.path() / "invalid.json";
    popnet_test::writeFile(invalid_path, json.dump());
    popnet_test::Arguments invalid_args{"test", "-JSON", invalid_path.string()};
    POPNET_EXPECT_THROW(std::invalid_argument,
                        Config invalid(invalid_args.argc(), invalid_args.argv()));

    json["vertices"] = 2;
    json["dimension"] = 5;
    json["routing_algorithm"] = static_cast<long>(RoutingType::CHIPLET_ROUTING_MESH);
    popnet_test::writeFile(invalid_path, json.dump());
    POPNET_EXPECT_THROW(std::invalid_argument,
                        Config invalid_chiplet(invalid_args.argc(), invalid_args.argv()));
}

void delayWriterPreservesTextFormats() {
    popnet_test::TempDirectory temp("delay-writer");
    const auto output_path = temp.path() / "delay.txt";
    {
        DelayWriter writer(output_path.string());
        Flit flit(1, FlitType::SINGLE, AddrType{0}, AddrType{1}, 3.75, DataType{1}, 5);
        writer.writePacket(flit, 4.25);

        ProtoPacket packet(1);
        packet.id = 6;
        packet.src_time = 5.8;
        packet.des_time = 7;
        packet.src_addr = {1};
        packet.des_addr = {0};
        packet.packet_size = 1;
        packet.proto_dsc = SPD_LAUNCH;
        ProtoStateMachine transaction(packet);
        writer.writeProtocolInjection(transaction, 2.5);
        transaction.packetDelay = {3.7, 5.2};
        writer.writeProtocolCompletion(transaction);
        writer.flush();
    }

    POPNET_CHECK_EQ(popnet_test::readFile(output_path), std::string("3 0 1 4.25\n"
                                                                    "5 1 0 65536 2 2.5 -1\n"
                                                                    "5 1 0 65536 2 3 5\n"));
}

void topologyAndReconfigurationBuildValidRoutingTables() {
    TopoInfo topology((fixture_dir / "line_3.gv").string());
    POPNET_CHECK_EQ(topology.vertexCnt, TAddressNumber{3});
    POPNET_CHECK_EQ(topology.vPortCount, std::vector<std::size_t>({2, 3, 2}));
    POPNET_CHECK(topology.routingPeriods.contains(2));
    POPNET_CHECK_EQ(topology.routingTable[0][2], 1);
    POPNET_CHECK_NEAR(topology.delayTable[0][2], 34, 1e-12);

    ReconfigTopoInfo reconfig((fixture_dir / "line_3.gv").string(),
                              (fixture_dir / "line_3.reconfig").string());
    POPNET_CHECK_EQ(reconfig.getCurrentReconfigurationPeriod(), std::int64_t{0});
    POPNET_CHECK(reconfig.hasNextReconfiguration());
    POPNET_CHECK_EQ(reconfig.routingTable[0][2], 1);
    reconfig.reconfigurate(0, 10);
    POPNET_CHECK_EQ(reconfig.getCurrentReconfigurationPeriod(), std::int64_t{1});
    POPNET_CHECK_EQ(reconfig.routingTable[0][2], 2);
    POPNET_CHECK_EQ(reconfig.old_routingTable[0][2], 1);
    POPNET_CHECK_NEAR(reconfig.nextReconfigurationTime, 10, 1e-12);
    reconfig.reconfigurate(10, 20);
    POPNET_CHECK(!reconfig.hasNextReconfiguration());
    POPNET_CHECK_EQ(reconfig.routingTable[0][2], 1);
}

void simulationStateIsIsolatedAndMasksAreBounded() {
    SimulationState first(3, 2, 7);
    SimulationState second(3, 2, 7);
    first.setCurrentTime(9);
    first.markFinished();
    POPNET_CHECK_NEAR(first.currentTime(), 9, 1e-12);
    POPNET_CHECK_EQ(first.totalFinished(), std::size_t{1});
    POPNET_CHECK_EQ(first.totalResolved(), std::size_t{1});
    POPNET_CHECK_NEAR(second.currentTime(), 0, 1e-12);
    POPNET_CHECK_EQ(second.totalFinished(), std::size_t{0});
    POPNET_CHECK(first.markAbandoned(42));
    POPNET_CHECK(!first.markAbandoned(42));
    POPNET_CHECK(first.isAbandoned(42));
    POPNET_CHECK_EQ(first.totalAbandoned(), std::size_t{1});
    POPNET_CHECK_EQ(first.totalResolved(), std::size_t{2});
    POPNET_CHECK_EQ(first.vcMasks.size(), std::size_t{6});
    POPNET_CHECK_EQ(first.random.random_long(0, 1000), second.random.random_long(0, 1000));
    POPNET_EXPECT_THROW(std::invalid_argument, SimulationState invalid(33, 2, 1));
}

void packetLossResolvesPacketsAndReturnsCredits() {
    popnet_test::TempDirectory temp("packet-loss-unit");
    const auto config_path = temp.path() / "config.json";
    const auto delay_path = temp.path() / "delay.txt";
    nlohmann::json json{
        {"vertices", 2},
        {"dimension", 1},
        {"vc_cnt", 2},
        {"input_buffer", 1},
        {"output_buffer", 1},
        {"flit_size", 1},
        {"link_length", 1},
        {"time", 20},
        {"random_seed", 1},
        {"routing_algorithm", static_cast<long>(RoutingType::XY)},
        {"trace_file", (fixture_dir / "empty.trace").string()},
        {"log_file", (temp.path() / "test.log").string()},
        {"delay_file", delay_path.string()},
        {"packet_loss", true},
    };
    popnet_test::writeFile(config_path, json.dump());
    popnet_test::Arguments arguments{"test", "-JSON", config_path.string()};
    Config config(arguments.argc(), arguments.argv());

    SimulationState state(config.getPhysicalPortNumber(), config.getVirtualChannelNumber(),
                          config.getRandomSeed());
    InputTrace trace((fixture_dir / "empty.trace").string(), false, 1, state.protocols);
    DelayWriter delay_writer(delay_path.string());
    RouterList routers;
    BaseRouter router(config, AddrType{1}, routers, state, trace, delay_writer);

    Flit buffered(0, FlitType::SINGLE, AddrType{0}, AddrType{1}, 0, DataType{1}, 10);
    buffered.setCreditReturn(AddrType{0}, 2);
    router.recvFlit(1, 0, std::move(buffered));
    POPNET_CHECK_EQ(state.totalResolved(), std::size_t{0});

    Flit dropped(1, FlitType::SINGLE, AddrType{0}, AddrType{1}, 0, DataType{2}, 11);
    dropped.setCreditReturn(AddrType{0}, 2);
    router.recvFlit(1, 0, std::move(dropped));
    POPNET_CHECK_EQ(state.totalAbandoned(), std::size_t{1});
    POPNET_CHECK_EQ(state.totalResolved(), std::size_t{1});
    const auto credit = state.events.popMessage();
    POPNET_CHECK_EQ(credit.getEventType(), MessType::CREDIT);
    POPNET_CHECK_EQ(credit.getDes(), AddrType{0});
    POPNET_CHECK_EQ(credit.getPC(), 2L);
    POPNET_CHECK_EQ(credit.getVC(), 0L);

    Flit same_packet(2, FlitType::TAIL, AddrType{0}, AddrType{1}, 0, DataType{3}, 11);
    same_packet.setCreditReturn(AddrType{0}, 2);
    router.recvFlit(1, 0, std::move(same_packet));
    POPNET_CHECK_EQ(state.totalAbandoned(), std::size_t{1});
    POPNET_CHECK_EQ(state.events.popMessage().getEventType(), MessType::CREDIT);
}

} // namespace

int main() {
    Logger::setLogLevel(LogLevel::Error);
    Logger::setLoggerOut(Logger::stdout_sink);
    return popnet_test::run({
        {"stable event queue", eventQueueIsStableAndPreservesPeriods},
        {"trace parsing", normalAndProtocolTracesKeepSourceOrdering},
        {"protocol lookup", protocolLookupIsIndexedAndRejectsDuplicates},
        {"flat router buffers", flatRouterBuffersPreserveCreditsAndMoves},
        {"bounded Orion wrapper", powerWrapperUsesBoundedStorage},
        {"configuration", configurationParsesJsonAndAllFlagKinds},
        {"delay formats", delayWriterPreservesTextFormats},
        {"topology and reconfiguration", topologyAndReconfigurationBuildValidRoutingTables},
        {"isolated simulation state", simulationStateIsIsolatedAndMasksAreBounded},
        {"packet-loss resolution", packetLossResolvesPacketsAndReturnsCredits},
    });
}
