# include "preprocess/config.h"

using json = nlohmann::json;

void Config::fromJson(const std::string& fname) {
    std::ifstream ifs(fname);
    if (!ifs) {
        throw std::runtime_error("Cannot open config file: " + fname);
    }

    json j;
    ifs >> j;

    if (j.contains("vertices")) {
        this->ary_number_ = j["vertices"].get<long>();
    }
    if (j.contains("dimension")) {
        this->cube_number_ = j["dimension"].get<long>();
    }
    if (j.contains("vc_cnt")) {
        this->virtual_channel_number_ = j["vc_cnt"].get<long>();
    }
    if (j.contains("input_buffer")) {
        this->in_buffer_size_ = j["input_buffer"].get<long>();
    }
    if (j.contains("output_buffer")) {
        this->out_buffer_size_ = j["output_buffer"].get<long>();
    }
    if (j.contains("flit_size")) {
        this->flit_size_ = j["flit_size"].get<long>();
    }
    if (j.contains("link_length")) {
        this->link_length_ = j["link_length"].get<double>();
    }
    if (j.contains("time")) {
        this->sim_length_ = j["time"].get<TimeType>();
    }
    if (j.contains("trace_file")) {
        this->trace_fname_ = j["trace_file"].get<std::string>();
    }
    if (j.contains("routing_algorithm")) {
        std::string tmp;
        switch (j["routing_algorithm"].type()) {
            case json::value_t::number_integer:
                this->routing_alg_ = j["routing_algorithm"].get<long>();
                break;
            case json::value_t::string:
                tmp = j["routing_algorithm"].get<std::string>();
                if (tmp == "XY") {
                    this->routing_alg_ = 0;
                }
                else if (tmp == "TXY") {
                    this->routing_alg_ = 1;
                }
                else if (tmp == "CHIPLET_ROUTING_MESH") {
                    this->routing_alg_ = 2;
                }
                else if (tmp == "CHIPLET_STAR_TOPO_ROUTING") {
                    this->routing_alg_ = 3;
                }
                else if (tmp == "GRAPH_TOPO") {
                    this->routing_alg_ = 4;
                }
                else if (tmp == "RECONFIGURABLE_GRAPH_TOPO") {
                    this->routing_alg_ = 5;
                }
                else {
                    throw std::runtime_error("Invalid routing algorithm type");
                }
                break;
            default:
                throw std::runtime_error("Invalid routing algorithm type");
        }
    }
    if (j.contains("report_period")) {
        this->report_period_ = j["report_period"].get<TimeType>();
    }
    if (j.contains("topology_file")) {
        this->topo_file_path_ = j["topology_file"].get<std::string>();
    }
    if (j.contains("reconfig_file")) {
        this->reconfig_file_path_ = j["reconfig_file"].get<std::string>();
    }
    if (j.contains("log_file")) {
        this->log_fname_ = j["log_file"].get<std::string>();
    }
    if (j.contains("packet_loss")) {
        this->packet_loss_ = j["packet_loss"].get<bool>();
    }
    if (j.contains("delay_file")) {
        this->delay_fname_ = j["delay_file"].get<std::string>();
    }
    if (j.contains("protocol_enable")) {
        this->sync_protocol_enable_ = j["protocol_enable"].get<bool>();
    }
    if (j.contains("random_seed")) {
        this->random_seed_ = std::make_unique<long>(j["random_seed"].get<long>());
    }
    if (j.contains("end_with_-1")) {
        this->end_with_minus_1_ = j["end_with_-1"].get<bool>();
    }
}

void Config::fromCMD(int argc, char * const argv []) {
    std::string opt_str = "h:?:A:c:V:B:F:T:r:I:O:R:L:G:m:C:l:D:P:E";
    std::string usage = std::string("usage: ") + argv[0] + " [" + opt_str + "] \n";
    std::string help("h: help\n?: help\nA: array size\nc: cube dimension\nV: virtual channel number\nB: buffer size\nO: outbuffer size\nF: flit size\nL: link legnth\nT: simulation length\nI: trace file\nR: routing algorithm: 0-dimension 1-opty\n");
    while (true) {
        
        long ch = ::getopt(argc, argv, opt_str.c_str());
        
        if(ch == -1) {
            break;
        }

        switch (ch) {

            case 'h':
                throw std::runtime_error(help);
                break;

            case 'A':
                this->ary_number_ = std::stoi(optarg);
                break;

            case 'c':
                this->cube_number_ = std::stoi(optarg);
                break;

            case 'V':
                this->virtual_channel_number_ = std::stoi(optarg);
                break;

            case 'B':
                this->in_buffer_size_ = std::stoi(optarg);
                break;

            case 'F':
                this->flit_size_ = std::stoi(optarg);
                break;

            case 'L':
                this->link_length_ = std::stoi(optarg);
                break;

            case 'T':
                this->sim_length_ = std::stoul(optarg);
                break;

            case 'I':
                this->trace_fname_ = optarg;
                break;
        
            case 'O':
                this->out_buffer_size_ = std::stoi(optarg);
                break;

            case 'r':
                this->random_seed_ = std::make_unique<long>(std::stol(optarg));
                break;

            case 'R':
                this->routing_alg_ = std::stoi(optarg);
                break;

            case 'G':
                this->topo_file_path_ = optarg;
                break;

            case 'm':
                this->report_period_ = std::stoi(optarg);
                break;
            
            case 'C':
                this->reconfig_file_path_ = optarg;
                break;

            case 'l':
                this->packet_loss_ = true;
                break;

            case 'D':
                this->delay_fname_ = optarg;
                break;

            case 'P':
                this->sync_protocol_enable_ = true;
                break;
            
            case 'E':
                this->end_with_minus_1_ = true;
                break;

            case '?':
                throw std::runtime_error(help);
                break;

            default:
                throw std::runtime_error(usage);
                break;
        }
    }
}

/**
 * @brief The constructor for the Config class
 */
Config::Config(int argc, char * const argv [])
:   ary_number_(8),
	cube_number_(2),
	virtual_channel_number_(2),
	in_buffer_size_(64),
	out_buffer_size_(4),
	sim_length_(1000000000),
	trace_fname_(),
	routing_alg_(0),
	vc_share_(VCShareType::SHARE),
    report_period_(REPORT_PERIOD_),
    log_fname_(),
    delay_fname_(),
    packet_loss_(false),
    sync_protocol_enable_(false),
    end_with_minus_1_(false)
{
    std::string help("h: help\n?: help\nA: array size\nc: cube dimension\nV: virtual channel number\nB: buffer size\nO: outbuffer size\nF: flit size\nL: link legnth\nT: simulation length\nI: trace file\nR: routing algorithm: 0-dimension 1-opty\n");
    Sassert(argc > 1, help.c_str());

    if (argc == 3 && std::string(argv[1]) == "-JSON") {
        this->fromJson(std::string(argv[2]));
    }
    else {
        this->fromCMD(argc, argv);
    }
    
    this->physical_port_number_ = this->cube_number_ * 2 + 1;
    if (this->getRoutingAlg() == RoutingType::CHIPLET_STAR_TOPO_ROUTING) {
        this->physical_port_number_ = std::max(this->physical_port_number_,
            this->getCubeNumber() * this->getCubeNumber() + 1);
    }
    
    if (this->log_fname_.empty()) {
        this->log_fname_ = std::string("logs/") + Logger::getCurrentTime() + ".log";
        if (!std::filesystem::exists("logs")) {
            std::filesystem::create_directories("logs");
        }
    }
    if (this->delay_fname_.empty()) {
        this->delay_fname_ = std::string("logs/") + Logger::getCurrentTime() + ".delayinfo.txt";
        if (!std::filesystem::exists("logs")) {
            std::filesystem::create_directories("logs");
        }
    }
    
}

long Config::getAryNumber() const {
    return this->ary_number_;
}

long Config::getCubeNumber() const {
    return this->cube_number_;
}

long Config::getVirtualChannelNumber() const {
    return this->virtual_channel_number_;
}

long Config::getPhysicalPortNumber() const {
    return this->physical_port_number_;
}

long Config::getInBufferSize() const {
    return this->in_buffer_size_;
}

long Config::getOutBufferSize() const {
    return this->out_buffer_size_;
}

long Config::getFlitSize() const {
    return this->flit_size_;
}

double Config::getLinkLength() const {
    return this->link_length_;
}

TimeType Config::getSimLength() const {
    return this->sim_length_;
}

const std::string& Config::getTraceFname() const {
    return this->trace_fname_;
}

RoutingType Config::getRoutingAlg() const {
    return RoutingType(this->routing_alg_);
}

VCShareType Config::getVcShare() const {
    return this->vc_share_;
}

const std::string& Config::getLogFilePath() const {
    return this->log_fname_;
}

const std::string& Config::getTopoFilePath() const {
    return this->topo_file_path_;
}

TimeType Config::getReportPeriod() const {
    return this->report_period_;
}

const std::string& Config::getReconfigFilePath() const {
    return this->reconfig_file_path_;
}

bool Config::isPacketLoss() const {
    return this->packet_loss_;
}

const std::string& Config::getDelayFname() const {
    return this->delay_fname_;
}

bool Config::isSyncProtocolEnable() const {
    return this->sync_protocol_enable_;
}

std::optional<long> Config::getRandomSeed() const {
    if (!this->random_seed_) {
        return std::nullopt;
    }
    return *this->random_seed_.get();
}

bool Config::isEndWithMinus1() const {
    return this->end_with_minus_1_;
}

std::ostream& operator<<(std::ostream& os, const Config& cf) {
    os << "Ary size:          " << cf.getAryNumber() << "\n";
    os << "Cube dimension:    " << cf.getCubeNumber() << "\n";
    os << "VC number:         " << cf.getVirtualChannelNumber() << "\n";
    os << "Inbuffer size:     " << cf.getInBufferSize() << "\n";
    os << "Outbuffer size:    " << cf.getOutBufferSize() << "\n";
    os << "Flit size:         " << cf.getFlitSize() << "\n";
    os << "Link length:       " << cf.getLinkLength() << "\n";
    os << "Simulation length: " << cf.getSimLength() << "\n";
    os << "Trace file:        " << cf.getTraceFname() << "\n";
    os << "Routing algorithm: " << cf.getRoutingAlg();
    return os;
}