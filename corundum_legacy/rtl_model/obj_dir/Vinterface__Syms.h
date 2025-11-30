// Verilated -*- C++ -*-
// DESCRIPTION: Verilator output: Symbol table internal header
//
// Internal details; most calling programs do not need this header

#ifndef _Vinterface__Syms_H_
#define _Vinterface__Syms_H_

#include "verilated_heavy.h"

// INCLUDE MODULE CLASSES
#include "Vinterface.h"

// SYMS CLASS
class Vinterface__Syms : public VerilatedSyms {
  public:
    
    // LOCAL STATE
    const char* __Vm_namep;
    bool __Vm_didInit;
    
    // SUBCELL STATE
    Vinterface*                    TOPp;
    
    // SCOPE NAMES
    VerilatedScope __Vscope_interface__axil_interconnect_inst;
    VerilatedScope __Vscope_interface__port__BRA__0__KET____port_inst__axil_interconnect_inst;
    
    // CREATORS
    Vinterface__Syms(Vinterface* topp, const char* namep);
    ~Vinterface__Syms() {}
    
    // METHODS
    inline const char* name() { return __Vm_namep; }
    
} VL_ATTR_ALIGNED(64);

#endif // guard
