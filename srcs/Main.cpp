
# include <exception>

# include "global.h"
# include "preprocess/config.h"
# include "sim/Sim.h"

int main(int argc, char *argv []) {
    Logger::setLogLevel(LogLevel::Debug);
    try {
		Config config(argc, argv);
        Logger::setLoggerOut(config.getLogFilePath());
		Logger::info(fmt::runtime(std::string("\n") + Logger::stream_to_string<Config>(config)));
		Sim sim(config);
		sim.mainProcess();
		sim.getResults();
	} catch (std::exception& e) {
        Logger::error("Exception: {}", e.what());
		std::cerr << e.what() << std::endl;
	}
    return 0;
}
