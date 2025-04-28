# pragma once

# ifndef _SIM_H_
# define _SIM_H_ 1

# include "global.h"
# include "preprocess/config.h"
# include "router/base_router.h"
# include "router/router.h"
# include "router/topo_router.h"

class Sim {

private:

    const Config& config_;

    TimeType last_time_;
	
    std::size_t mess_count_;

    std::size_t router_count_;

    std::vector<BaseRouter *> inter_network_;

    struct {
        TopoInfo *topo_info = nullptr;
        ReconfigTopoInfo *reconfig_topo_info = nullptr;
    } topo_info;

    void setInitEvent();

    void receive_EVG_message(MessEvent mesg);
    
    void receive_ROUTER_message(MessEvent mesg);

    void receive_WIRE_message(MessEvent mesg);

    void receive_CREDIT_message(MessEvent mesg);

    void receive_RECONFIGURATION_message(MessEvent mesg);

    BaseRouter& router(const AddrType& addr);

public:
    
    Sim(const Config& config);
    
    ~Sim();
    
    void mainProcess();
    
    void getResults();

};


# endif