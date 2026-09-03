#include "test_support.h"

#include <filesystem>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>

#include "logger/logger.hpp"
#include "preprocess/config.h"
#include "sim/Sim.h"

namespace {

const std::filesystem::path fixture_dir{POPNET_TEST_FIXTURES};

struct RunOutcome {
    SimulationResults results;
    std::string delay_output;
};

RunOutcome runSimulation(const std::filesystem::path& directory, std::string_view name,
                         const std::filesystem::path& trace, long vertices, long dimension,
                         RoutingType routing, TimeType time_limit = 200, bool protocol = false,
                         bool packet_loss = false, const std::filesystem::path& topology = {},
                         const std::filesystem::path& reconfiguration = {}) {
    const auto prefix = directory / std::string(name);
    const auto config_path = prefix.string() + ".json";
    const auto log_path = prefix.string() + ".log";
    const auto delay_path = prefix.string() + ".delay";

    nlohmann::json json{
        {"vertices", vertices},
        {"dimension", dimension},
        {"vc_cnt", 2},
        {"input_buffer", 12},
        {"output_buffer", 12},
        {"flit_size", 2},
        {"link_length", 1000},
        {"time", time_limit},
        {"random_seed", 1},
        {"routing_algorithm", static_cast<long>(routing)},
        {"trace_file", trace.string()},
        {"log_file", log_path},
        {"delay_file", delay_path},
        {"protocol_enable", protocol},
        {"packet_loss", packet_loss},
        {"end_with_-1", false},
    };
    if (!topology.empty()) {
        json["topology_file"] = topology.string();
    }
    if (!reconfiguration.empty()) {
        json["reconfig_file"] = reconfiguration.string();
    }
    popnet_test::writeFile(config_path, json.dump());

    popnet_test::Arguments arguments{"test", "-JSON", config_path};
    Config config(arguments.argc(), arguments.argv());
    Logger::setLoggerOut(log_path);

    RunOutcome outcome;
    {
        Sim simulation(config);
        simulation.mainProcess();
        outcome.results = simulation.results();
    }
    Logger::setLoggerOut(Logger::stdout_sink);
    outcome.delay_output = popnet_test::readFile(delay_path);
    return outcome;
}

void fixedSeedGoldenRunIsRepeatable() {
    popnet_test::TempDirectory temp("golden");
    const auto first =
        runSimulation(temp.path(), "first", fixture_dir / "single_xy.trace", 2, 2, RoutingType::XY);
    const auto second = runSimulation(temp.path(), "second", fixture_dir / "single_xy.trace", 2, 2,
                                      RoutingType::XY);

    POPNET_CHECK_EQ(first.results.total_finished, std::size_t{1});
    POPNET_CHECK_NEAR(first.results.average_delay, 6, 1e-12);
    POPNET_CHECK_EQ(first.delay_output, std::string("0 0 0 1 0 6\n"));
    POPNET_CHECK_EQ(second.delay_output, first.delay_output);
    POPNET_CHECK_NEAR(second.results.total_power, first.results.total_power, 1e-12);
}

void sourceLocalTimingDoesNotInjectFuturePackets() {
    popnet_test::TempDirectory temp("source-local");
    const auto outcome = runSimulation(
        temp.path(), "source-local", fixture_dir / "source_local.trace", 2, 1, RoutingType::XY, 50);
    POPNET_CHECK_EQ(outcome.results.total_finished, std::size_t{2});
    POPNET_CHECK_EQ(popnet_test::lineCount(outcome.delay_output), std::size_t{2});
}

void emptyTraceProducesFiniteZeroResults() {
    popnet_test::TempDirectory temp("empty");
    const auto outcome =
        runSimulation(temp.path(), "empty", fixture_dir / "empty.trace", 2, 1, RoutingType::XY, 10);
    POPNET_CHECK_EQ(outcome.results.total_finished, std::size_t{0});
    POPNET_CHECK_NEAR(outcome.results.average_delay, 0, 1e-12);
    POPNET_CHECK_NEAR(outcome.results.total_power, 0, 1e-12);
    POPNET_CHECK(outcome.delay_output.empty());
}

void allRoutingModesCompleteARepresentativePacket() {
    popnet_test::TempDirectory temp("routing-modes");

    const auto txy =
        runSimulation(temp.path(), "txy", fixture_dir / "single_xy.trace", 2, 2, RoutingType::TXY);
    POPNET_CHECK_EQ(txy.results.total_finished, std::size_t{1});

    const auto chiplet_mesh =
        runSimulation(temp.path(), "chiplet-mesh", fixture_dir / "single_chiplet.trace", 2, 4,
                      RoutingType::CHIPLET_ROUTING_MESH);
    POPNET_CHECK_EQ(chiplet_mesh.results.total_finished, std::size_t{1});

    const auto chiplet_star =
        runSimulation(temp.path(), "chiplet-star", fixture_dir / "single_chiplet.trace", 2, 4,
                      RoutingType::CHIPLET_STAR_TOPO_ROUTING);
    POPNET_CHECK_EQ(chiplet_star.results.total_finished, std::size_t{1});

    const auto graph =
        runSimulation(temp.path(), "graph", fixture_dir / "single_graph.trace", 3, 1,
                      RoutingType::GRAPH_TOPO, 200, false, false, fixture_dir / "line_3.gv");
    POPNET_CHECK_EQ(graph.results.total_finished, std::size_t{1});

    const auto reconfig =
        runSimulation(temp.path(), "reconfig", fixture_dir / "single_graph.trace", 3, 1,
                      RoutingType::RECONFIGURABLE_GRAPH_TOPO, 200, false, false,
                      fixture_dir / "line_3.gv", fixture_dir / "line_3.reconfig");
    POPNET_CHECK_EQ(reconfig.results.total_finished, std::size_t{1});
}

void protocolTrafficCompletesDataAndAcknowledgement() {
    popnet_test::TempDirectory temp("protocol");
    const auto outcome = runSimulation(temp.path(), "protocol", fixture_dir / "launch.trace", 2, 1,
                                       RoutingType::XY, 200, true);
    POPNET_CHECK_EQ(outcome.results.total_finished, std::size_t{2});
    POPNET_CHECK_EQ(popnet_test::lineCount(outcome.delay_output), std::size_t{3});
    POPNET_CHECK(outcome.delay_output.find("65536") != std::string::npos);
}

void packetLossModeKeepsTheSameExternalFormats() {
    popnet_test::TempDirectory temp("packet-loss");
    const auto outcome = runSimulation(temp.path(), "packet-loss", fixture_dir / "single_xy.trace",
                                       2, 2, RoutingType::XY, 200, false, true);
    POPNET_CHECK_EQ(outcome.results.total_finished, std::size_t{1});
    POPNET_CHECK_EQ(popnet_test::lineCount(outcome.delay_output), std::size_t{1});
}

void simulationInstanceCannotRunTwice() {
    popnet_test::TempDirectory temp("run-once");
    const auto config_path = temp.path() / "config.json";
    nlohmann::json json{
        {"vertices", 2},
        {"dimension", 2},
        {"vc_cnt", 2},
        {"input_buffer", 12},
        {"output_buffer", 12},
        {"flit_size", 2},
        {"link_length", 1000},
        {"time", 20},
        {"random_seed", 1},
        {"routing_algorithm", static_cast<long>(RoutingType::XY)},
        {"trace_file", (fixture_dir / "single_xy.trace").string()},
        {"log_file", (temp.path() / "test.log").string()},
        {"delay_file", (temp.path() / "test.delay").string()},
    };
    popnet_test::writeFile(config_path, json.dump());
    popnet_test::Arguments arguments{"test", "-JSON", config_path.string()};
    Config config(arguments.argc(), arguments.argv());
    Sim simulation(config);
    simulation.mainProcess();
    POPNET_EXPECT_THROW(std::logic_error, simulation.mainProcess());
}

} // namespace

int main() {
    Logger::setLogLevel(LogLevel::Error);
    Logger::setLoggerOut(Logger::stdout_sink);
    return popnet_test::run({
        {"fixed-seed golden run", fixedSeedGoldenRunIsRepeatable},
        {"source-local timing", sourceLocalTimingDoesNotInjectFuturePackets},
        {"empty results", emptyTraceProducesFiniteZeroResults},
        {"all routing modes", allRoutingModesCompleteARepresentativePacket},
        {"protocol traffic", protocolTrafficCompletesDataAndAcknowledgement},
        {"packet-loss mode", packetLossModeKeepsTheSameExternalFormats},
        {"single-use simulation", simulationInstanceCannotRunTwice},
    });
}
