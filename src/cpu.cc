#include "cpu.h"

namespace dramsim3 {

void RandomCPU::ClockTick() {
    // Create random CPU requests at full speed
    // this is useful to exploit the parallelism of a DRAM protocol
    // and is also immune to address mapping and scheduling policies
  // TODO: Prio passing here? Can just do random since this is random anyway
    memory_system_.ClockTick();
    if (get_next_) {
        last_addr_ = gen();
        last_write_ = (gen() % 3 == 0);
    }
    bool default_prio = false;
    get_next_ = memory_system_.WillAcceptTransaction(last_addr_, last_write_, default_prio);
    if (get_next_) {
        memory_system_.AddTransaction(last_addr_, last_write_, default_prio);
    }
    clk_++;
    return;
}

void StreamCPU::ClockTick() {
    // stream-add, read 2 arrays, add them up to the third array
    // this is a very simple approximate but should be able to produce
    // enough buffer hits

    // moving on to next set of arrays
    // TODO: Check how to pass prio here. Is it needed? Or can I ignore since its
    // streamcpu not trace
    memory_system_.ClockTick();
    if (offset_ >= array_size_ || clk_ == 0) {
        addr_a_ = gen();
        addr_b_ = gen();
        addr_c_ = gen();
        offset_ = 0;
    }

  bool default_prio = false;
    if (!inserted_a_ &&
        memory_system_.WillAcceptTransaction(addr_a_ + offset_, false, default_prio)) {
        memory_system_.AddTransaction(addr_a_ + offset_, false, default_prio);
        inserted_a_ = true;
    }
    if (!inserted_b_ &&
        memory_system_.WillAcceptTransaction(addr_b_ + offset_, false, default_prio)) {
        memory_system_.AddTransaction(addr_b_ + offset_, false, default_prio);
        inserted_b_ = true;
    }
    if (!inserted_c_ &&
        memory_system_.WillAcceptTransaction(addr_c_ + offset_, true, default_prio)) {
        memory_system_.AddTransaction(addr_c_ + offset_, true, default_prio);
        inserted_c_ = true;
    }
    // moving on to next element
    if (inserted_a_ && inserted_b_ && inserted_c_) {
        offset_ += stride_;
        inserted_a_ = false;
        inserted_b_ = false;
        inserted_c_ = false;
    }
    clk_++;
    return;
}

TraceBasedCPU::TraceBasedCPU(const std::string& config_file,
                             const std::string& output_dir,
                             const std::string& trace_file)
    : CPU(config_file, output_dir) {
    trace_file_.open(trace_file);
    if (trace_file_.fail()) {
        std::cerr << "Trace file does not exist" << std::endl;
        AbruptExit(__FILE__, __LINE__);
    }
}

void TraceBasedCPU::ClockTick() {
    memory_system_.ClockTick();
    if (!trace_file_.eof()) {
        if (get_next_) {
            get_next_ = false;
            trace_file_ >> trans_;
        }
        if (trans_.added_cycle <= clk_) {
            get_next_ = memory_system_.WillAcceptTransaction(trans_.addr,
                                                             trans_.is_write,
                                                             trans_.is_prio);
            if (get_next_) {
                memory_system_.AddTransaction(trans_.addr, trans_.is_write, trans_.is_prio);
            }
        }
    }
    clk_++;
    return;
}

}  // namespace dramsim3
